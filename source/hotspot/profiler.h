/*
 * profiler.h
 *
 *  Created on: 2013-02-23
 *      Author: sam
 */

#ifndef PROFILER_H_
#define PROFILER_H_

#include <map>
#include <string>
#include <vector>
#include <thread>
#include <ctime>
#include <mutex>


#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Function.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "../functions.h"
#include "../arrayobj.h"
#include "../objects.h"
#include "../typeinfer.h"
#include "../loopstmts.h"

namespace hotspot
{

class FunctionSignature
{
public:
  FunctionSignature(
      const Function* caller, const TypeSetString& callerArgs,
      const Function* callee, const TypeSetString& calleeArgs);
  ~FunctionSignature();

  bool operator==(const FunctionSignature& another) const;
  bool operator<(const FunctionSignature& another) const;

  const std::string& toString() const;

private:
  std::string name;
};

class LoopSignature
{
public:
  LoopSignature(
      const Function* func, const TypeSetString& funcArgs,
      LoopStmt* loop);
  ~LoopSignature();

  bool operator==(const LoopSignature& another) const;
  bool operator<(const LoopSignature& another) const;

  const std::string& toString() const;

private:
  std::string name;
};


/**
 * NOTE: using std::map because it guarantees validity of pointer even after
 * insertion
 */
typedef std::map<FunctionSignature, unsigned int>
  FunctionCounter;

/**
 * similar to FunctionCounter, and this is for counting loop iterations
 */
typedef std::map<LoopSignature, unsigned int>
  LoopCounter;

class Profiler
{
public:
  virtual ~Profiler();

  static Profiler* get();
  static void registerConfigVars();

  /**
   * initialize the profiler to work on given engine and context
   * it's never valid to call this method twice
   */
  void initialize(llvm::ExecutionEngine* engine, llvm::Module* module);

  void instrumentFuncCall(
      const FunctionSignature& sig, llvm::BasicBlock* entryBlock);

  void instrumentLoopIter(
      const LoopSignature& sig, llvm::BasicBlock* loopBody);

  /**
   * shutdown background threads
   */
  void shutdown();

private:
  Profiler();
  FunctionCounter m_func_counts;
  LoopCounter m_loop_counts;

  llvm::Function* m_llvmfunc = NULL;

  /**
   * same as notify, but called from LLVM's compiled code
   */
  static void notify(unsigned int* addr);

  /*
   * worker related stuff
   */
  void maintain();
  void decay();

  /**
   * dump the counter information (csv with header)
   * to counters.out file in the current folder
   */
  void dump();

  std::thread m_worker;
  volatile bool m_shutdown = false;


};

} /* namespace hotspot */
#endif /* PROFILER_H_ */
