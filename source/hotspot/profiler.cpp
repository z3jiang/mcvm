/*
 * profiler.cpp
 *
 *  Created on: 2013-02-23
 *      Author: sam
 */

#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <climits>

#include <llvm/Function.h>
#include <llvm/IRBuilder.h>
#include <llvm/Module.h>

#include "profiler.h"
#include "../utils/llvmutils.h"
#include "../debug.h"
#include "../configmanager.h"

#define WORKER_THREAD_SLEEP 1000

using namespace std;
using namespace std::chrono;
using namespace LLVMUtils;

namespace hotspot
{

typedef std::vector<std::string> ArgNames;
typedef std::vector<llvm::Type*> ArgTypes;

ConfigVar cfgWorkerSleepMS("pWorkerSleepMS", ConfigVar::INT, "1000");
ConfigVar cfgProfileFuncs("pFuncCalls", ConfigVar::BOOL, "true");
ConfigVar cfgProfileLoops("pLoops", ConfigVar::BOOL, "true");
ConfigVar cfgProfileInterpreter("pInterpreter", ConfigVar::BOOL, "true");

void Profiler::registerConfigVars()
{
  ConfigManager::registerVar(&cfgWorkerSleepMS);
  ConfigManager::registerVar(&cfgProfileFuncs);
  ConfigManager::registerVar(&cfgProfileLoops);
}


void Profiler::maintain()
{
  DEBUG("profiler worker thread starting on " << this << endl);

  try
  {
    while (!m_shutdown)
    {
      this_thread::sleep_for( milliseconds(cfgWorkerSleepMS.getIntValue()) );

      steady_clock::time_point t = steady_clock::now();

      DEBUG("profiler maintenance" << endl);
      decay();

      microseconds elapsed = duration_cast<microseconds>(
          (steady_clock::now().time_since_epoch() - t.time_since_epoch()));

      DEBUG("profiler maintenance took " << elapsed.count() << "us" << endl);
    }
  }
  catch (...)
  {
    cerr << "Profiler worker thread caught unexpected exception " << endl;
  }

  DEBUG("profiler worker thread on " << this << " exited" << endl);
}

Profiler* Profiler::get()
{
  static Profiler profiler;
  return &profiler;
}

void buildIncr(void* valueptr, llvm::BasicBlock* bb)
{
  llvm::IRBuilder<> builder(bb);

  llvm::Type* i32 = getIntType(4);

  llvm::Value* curValue =
      builder.CreateLoad(createPtrConst(valueptr, i32), true);
  llvm::Value* newValue =
      builder.CreateAdd(curValue, llvm::ConstantInt::get(i32, 1));
  builder.CreateStore(newValue,
      createPtrConst(valueptr, i32), true);
}

void Profiler::instrumentFuncCall(
    const FunctionSignature& sig, llvm::BasicBlock* entryBlock)
{
  if (!cfgProfileFuncs.getBoolValue())
  {
    return;
  }

  if (m_func_counts.find(sig) == m_func_counts.end())
  {
    m_func_counts[sig] = 0;
  }

  // see m_counter type for why this is possible
  unsigned int* value = &(m_func_counts[sig]);

  DEBUG("Will instrument function call "
      << sig.toString() << " at " << value << endl);

  buildIncr(value, entryBlock);
}

void Profiler::instrumentLoopIter(
    const LoopSignature& sig, llvm::BasicBlock* loopBody)
{
  if (!cfgProfileLoops.getBoolValue())
  {
    return;
  }

  if (m_loop_counts.find(sig) == m_loop_counts.end())
  {
    m_loop_counts[sig] = 0;
  }

  // see type for why this is possible
  unsigned int* value = &(m_loop_counts[sig]);

  DEBUG("Will instrument loop iteration "
      << sig.toString() << " at " << value << endl);

  buildIncr(value, loopBody);
}


void dumpOne(ostream& s, const Counters& c)
{
  for (Counters::const_iterator i = c.begin();
       i != c.end();
       i++)
  {
    s << i->first.toString() << "," << i->second << endl;
  }
}

void Profiler::dump()
{
  // dump out counter information
  // read only is safe without lock
  ofstream out("./counters.out");
  if (!out.good())
  {
    cerr << "Could not open " << out << endl;
    return;
  }

  // header. the external visualizer depends on the header names
  out << "calling,callee,count" << endl;

  dumpOne(out, m_func_counts);
  dumpOne(out, m_loop_counts);
  dumpOne(out, m_itpr_counts);

  out.close();
  DEBUG("counters.out written" << endl);
}

unsigned int max(Counters& counters)
{
  unsigned int ret = 0;

  for (Counters::const_iterator i = counters.begin();
       i != counters.end();
       i++)
  {
    if (i->second > ret)
    {
      ret = i->second;
    }
  }

  return ret;
}

/**
 * reduce each element in the counters map to the given percentage, e.g.
 * new = old * percentage
 */
void reduce(Counters& counters, float percentage)
{
  for (Counters::iterator i = counters.begin();
       i != counters.end();
       i++)
  {
    unsigned int newVal = i->second * percentage;

    // on x86 or x64, store of 32 bit values is atomic, but the entire
    // read-incr-write operation from llvm thread is not atomic. when decaying,
    // we don't care about exact new values, as long as all counters go down
    // by roughly the same amount, so we don't create artificial hotspots.
    // By doing this assignment to new value a few times, we shouldn't need
    // lock and we would achieve the same desired decay effect.
    i->second = newVal;
    i->second = newVal;
    i->second = newVal;
    i->second = newVal;
    i->second = newVal;
  }
}

void decayCounter(Counters& counters)
{
  unsigned int countermax = max(counters);

  // TODO what's a good reduction? should it be more dynamic?

  if (countermax > INT32_MAX / 2)
  {
    // one important thing is to decay extra aggressively
    // if we are close to overflow
    reduce(counters, 0.7);
  }
  else
  {
    reduce(counters, 0.9);
  }
}

void Profiler::decay()
{
  decayCounter(m_func_counts);
  decayCounter(m_loop_counts);
  decayCounter(m_itpr_counts);
}

Profiler::Profiler()
{
  // spin off worker thread
  m_worker = thread(bind(&Profiler::maintain, this));
}

Profiler::~Profiler()
{
  if (!m_shutdown)
  {
    cerr << "BUG: did not shutdown profiler background thread" << endl;
    shutdown();
  }
}


string genTypeSignature(const TypeSetString& args)
{
  stringstream ss;
  ss << "(";

  for (TypeSetString::const_iterator i = args.begin();
       i != args.end();
       i++)
  {
    TypeSet types = *i;
    // why is this a set? an arg can have possible sets of types?

    if (i != args.begin())
    {
      ss << ", ";
    }

    for (TypeSet::const_iterator j = types.begin();
         j != types.end();
         j++)
    {
      if (j != types.begin())
      {
        ss << "|";
      }

      // use short name. the full toString is too verbose
      ss << DataObject::getTypeName( j->getObjType() );

      if (j->isScalar())
      {
        ss << "S";
      }

      if (j->isInteger())
      {
        ss << "I";
      }

      if (j->is2D())
      {
        ss << "2";
      }
    }
  }

  ss << ")";
  return ss.str();
}

FunctionSignature::FunctionSignature(
    const Function* caller, const TypeSetString& callerArgs,
    const Function* callee, const TypeSetString& calleeArgs)
{
  // squash everything into csv fragment
  stringstream ss;
  ss << "\"";
  ss << caller->getFuncName();
  ss << genTypeSignature(callerArgs);

  ss << "\",\"";

  ss << callee->getFuncName() << genTypeSignature(calleeArgs);
  ss << "\"";

  m_signature = ss.str();
}

FunctionSignature::~FunctionSignature()
{
}

void Profiler::shutdown()
{
  DEBUG("shutting down worker thread" << endl);
  m_shutdown = true;

  DEBUG("waiting for worker to join" << endl);
  m_worker.join();

  // force a dump so we get the latest information
  dump();
}


LoopSignature::LoopSignature(
    const Function* func, const TypeSetString& funcArgs,
    LoopStmt* loop)
{
  stringstream ss;
  ss << "\"";
  ss << func->getFuncName();
  ss << genTypeSignature(funcArgs);

  ss << "\",\"";
  ss << "_loop" << loop << "\"";
  // TODO use better readable identification than address

  m_signature = ss.str();
}

LoopSignature::~LoopSignature()
{
}

bool Signature::operator ==(const Signature& another) const
{
  return m_signature == another.m_signature;
}

bool Signature::operator <(const Signature& another) const
{
  return m_signature < another.m_signature;
}

const std::string& Signature::toString() const
{
  return m_signature;
}


void Profiler::cInstrumentInterpreter(llvm::BasicBlock* bb)
{
  if (!cfgProfileInterpreter.getBoolValue())
  {
    return;
  }

  // let it bomb if stack is empty
  unsigned int* value = &(m_itpr_counts[m_contexts.top()]);
  DEBUG("Will instrument interpreted calls at " << value
      << " in context " << m_contexts.top().toString() << endl);

  buildIncr(value, bb);
}

void Profiler::cPushContext(const InterpretedCallSignature& sig)
{
  for (size_t i=0; i<m_contexts.size(); i++)
  {
    DEBUG("  ");
  }
  DEBUG("New context " << sig.toString() << endl);

  m_contexts.push(sig);
}

void Profiler::cPopContext()
{
  m_contexts.pop();

  for (size_t i=0; i<m_contexts.size(); i++)
  {
    DEBUG("  ");
  }
  DEBUG("Popped context" << endl);
}

void Profiler::cAssert() const
{
  if (!m_contexts.empty())
  {
    cerr << "BUG: profiler contexts not popped properly" << endl;
  }
}

InterpretedCallSignature::InterpretedCallSignature(
    const Function* ownerFunc, const TypeSetString& ownFuncArgs)
{
  stringstream ss;
  ss << "\"";
  ss << ownerFunc->getFuncName();
  ss << genTypeSignature(ownFuncArgs);

  ss << "\",\"_interpreted" << ownerFunc << "\"";

  m_signature = ss.str();
}

InterpretedCallSignature::InterpretedCallSignature(const LoopStmt* ownerLoop)
{
  stringstream ss;
  ss << "\"_loop" << ownerLoop;
  ss << "\",\"_interpreted" << ownerLoop << "\"";
  // TODO use better readable identification than address

  m_signature = ss.str();
}

InterpretedCallSignature::~InterpretedCallSignature()
{
}


} /* namespace hotspot */


