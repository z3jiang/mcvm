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

namespace hotspot
{

class FunctionSignature
{
public:
  FunctionSignature(const Function* caller, const Function* callee);
  ~FunctionSignature();

  std::string name;

  bool operator==(const FunctionSignature& another) const;
  bool operator<(const FunctionSignature& another) const;

  const std::string& toString() const;
};

/**
 * NOTE: using std::map because it guarantees validity of pointer even after
 * insertion
 */
typedef std::map<FunctionSignature, unsigned int>
  FunctionCounter;

class Profiler
{
public:
  virtual ~Profiler();

  static Profiler* get();

  /**
   * initialize the profiler to work on given engine and context
   * it's never valid to call this method twice
   */
  void initialize(llvm::ExecutionEngine* engine, llvm::Module* module);

  enum Strategy {
    OFF, BASIC
  };

  void instrumentFuncCall(Function* caller, Function* callee,
      llvm::BasicBlock* entryBlock);

  /**
   * shutdown background threads
   */
  void shutdown();

private:
  Profiler();
  Strategy m_strat = Strategy::BASIC;
  FunctionCounter m_counter;
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
  void dump();

  std::thread m_worker;
  std::mutex m_lock;
  volatile bool m_shutdown = false;
  time_t m_next_decay;


};

} /* namespace hotspot */
#endif /* PROFILER_H_ */
