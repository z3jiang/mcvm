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

#include <llvm/Function.h>
#include <llvm/IRBuilder.h>
#include <llvm/Module.h>

#include "profiler.h"
#include "../utils/llvmutils.h"
#include "../debug.h"

using namespace std;
using namespace LLVMUtils;

#define DECAY 0.9

namespace hotspot
{

typedef std::vector<std::string> ArgNames;
typedef std::vector<llvm::Type*> ArgTypes;



void Profiler::maintain()
{
  DEBUG("profiler worker thread starting on " << this << endl);

  try
  {
    while (!m_shutdown)
    {
      time_t now = time(NULL);
      if (now < m_next_decay)
      {
        this_thread::sleep_for( chrono::milliseconds(100) );
        // sleep short so we don't need to implement interrupt
        continue;
      }

      DEBUG("profiler maintenance" << endl);

      m_next_decay = now + 60; // minutely

      m_lock.lock();
      decay();
      // for now, not going to worry about number of functions grow too big
      m_lock.unlock();

      DEBUG("profiler maintenance took " << time(NULL) - now << endl);

      dump();
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
  if (!get()->m_lock.try_lock())
  {
    // we miss some updates, but better than waiting for lock
    return;
  }

  *valueaddr += 1;

  get()->m_lock.unlock();
}

Profiler* Profiler::get()
{
  static Profiler profiler;
  return &profiler;
}

void Profiler::instrumentFuncCall(Function* caller, Function* callee,
    llvm::BasicBlock* entryBlock)
{
  hotspot::FunctionSignature sig = FunctionSignature(caller, callee);

  if (m_counter.find(sig) == m_counter.end())
  {
    m_counter[sig] = 0;
  }

  // see m_counter type for why this is possible
  unsigned int* value = &(m_counter[sig]);

  DEBUG("Will instrument function call "
      << sig.toString() << " at " << value << endl);

  llvm::IRBuilder<> builder(entryBlock);
  builder.CreateCall(m_llvmfunc, createPtrConst(value));
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

  for (FunctionCounter::iterator i = m_counter.begin();
       i != m_counter.end();
       i++)
  {
    out << i->first.name << "," << i->second << endl;
  }

  out.close();
  DEBUG("counters.out written" << endl);
}

void Profiler::decay()
{
}

Profiler::Profiler()
:
  m_next_decay(time(NULL)) // we decay right away, doesn't really matter
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


FunctionSignature::FunctionSignature(
    const Function* caller, const Function* callee)
{
  name = caller == NULL ? "" : caller->getFuncName()
      + "," + callee->getFuncName();
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

} /* namespace hotspot */

