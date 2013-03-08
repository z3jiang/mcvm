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

void Profiler::notify(unsigned int* valueaddr)
{
  *valueaddr += 1;
}

Profiler* Profiler::get()
{
  static Profiler profiler;
  return &profiler;
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

  llvm::IRBuilder<> builder(entryBlock);

  llvm::Type* i32 = getIntType(4);

  llvm::Value* curValue =
      builder.CreateLoad(createPtrConst(value, i32), true);
  llvm::Value* newValue =
      builder.CreateAdd(curValue, llvm::ConstantInt::get(i32, 1));
  builder.CreateStore(newValue, createPtrConst(value, i32), true);

//  builder.CreateCall(m_llvmfunc, createPtrConst(value));
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

  llvm::IRBuilder<> builder(loopBody);

  llvm::Type* i32 = getIntType(4);

  llvm::Value* curValue =
      builder.CreateLoad(createPtrConst(value, i32), true);
  llvm::Value* newValue =
      builder.CreateAdd(curValue, llvm::ConstantInt::get(i32, 1));
  builder.CreateStore(newValue, createPtrConst(value, i32), true);

  // NOTE, if we are to support concurrent threads, use CreateAtomicRMW instead

//   builder.CreateCall(m_llvmfunc, createPtrConst(value));
}


void Profiler::initialize(llvm::ExecutionEngine* engine,
    llvm::Module* module)
{
  // define the instrumentation function and link to the desired static method

  llvm::IRBuilder<> builder(module->getContext());

  ArgTypes argTypes;
  argTypes.push_back(VOID_PTR_TYPE);

  llvm::FunctionType *functType = llvm::FunctionType::get(
      builder.getVoidTy(), argTypes, false);

  m_llvmfunc = llvm::Function::Create(functType,
      llvm::Function::ExternalLinkage, "_notify_profiler", module);

  engine->addGlobalMapping(m_llvmfunc, (void*) &Profiler::notify);
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

  for (FunctionCounter::iterator i = m_func_counts.begin();
       i != m_func_counts.end();
       i++)
  {
    out << i->first.toString() << "," << i->second << endl;
  }

  for (LoopCounter::iterator i = m_loop_counts.begin();
       i != m_loop_counts.end();
       i++)
  {
    out << i->first.toString() << "," << i->second << endl;
  }

  out.close();
  DEBUG("counters.out written" << endl);
}

template<class T>
unsigned int max(const map<T, unsigned int>& counters)
{
  unsigned int ret = 0;

  for (typename map<T, unsigned int>::const_iterator i = counters.begin();
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
template<class T>
void reduce(map<T, unsigned int>& counters, float percentage)
{
  for (typename map<T, unsigned int>::iterator i = counters.begin();
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

template<class T>
void decayCounter(map<T, unsigned int>& counters)
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

  name = ss.str();
}

FunctionSignature::~FunctionSignature()
{
}

bool FunctionSignature::operator ==(
    const FunctionSignature& another) const
{
  return name == another.name;
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

bool FunctionSignature::operator<(const FunctionSignature& another) const
{
  return name < another.name;
}

const std::string& FunctionSignature::toString() const
{
  return name;
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
  ss << "_loop0x" << loop << "\"";
  // TODO use better readable identification than address

  name = ss.str();
}

LoopSignature::~LoopSignature()
{
}

bool LoopSignature::operator ==(const LoopSignature& another) const
{
  return name == another.name;
}

bool LoopSignature::operator <(const LoopSignature& another) const
{
  return name < another.name;
}

const std::string& LoopSignature::toString() const
{
  return name;
}





} /* namespace hotspot */

