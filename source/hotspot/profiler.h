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
#include <stack>


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

/**
 * base abstract class for signatures. these are keys of counters
 */
class Signature
{
public:
  virtual ~Signature() {};

  virtual bool operator==(const Signature& another) const;
  virtual bool operator<(const Signature& another) const;

  virtual const std::string& toString() const;


protected:
  /**
   * implementation classes should squash everything into m_signature
   */
  Signature() {};

  std::string m_signature;
};

class FunctionSignature : public Signature
{
public:
  FunctionSignature(
      const Function* caller, const TypeSetString& callerArgs,
      const Function* callee, const TypeSetString& calleeArgs);
  virtual ~FunctionSignature();
};

class LoopSignature : public Signature
{
public:
  LoopSignature(
      const Function* func, const TypeSetString& funcArgs,
      LoopStmt* loop);
  virtual ~LoopSignature();
};

class InterpretedCallSignature : public Signature
{
public:
  InterpretedCallSignature(
      const Function* ownerFunc, const TypeSetString& ownFuncArgs);
  InterpretedCallSignature(
      const LoopStmt* ownerLoop);
  virtual ~InterpretedCallSignature();
};


/**
 * NOTE: using std::map because it guarantees validity of pointer even after
 * insertion
 */
typedef std::map<Signature, unsigned int> Counters;

/**
 * hotspot profiler
 *
 * instruments (counts) interesting aspects of a program, at runtime, with
 * minimal overhead
 *
 * interesting aspects are for example loop iterations, function calls
 *
 * see instrument* methods
 * see c* methods
 */
class Profiler
{
public:
  virtual ~Profiler();

  static Profiler* get();
  static void registerConfigVars();

  void instrumentFuncCall(
      const FunctionSignature& sig, llvm::BasicBlock* entryBlock);

  void instrumentLoopIter(
      const LoopSignature& sig, llvm::BasicBlock* loopBody);

  /**
   * contextual (stateful) call
   * instrument interpreter calls. these are known to be slow.
   * all calls will record into the current context. see c*Context calls
   *
   * NOTE: I'm not a big fan of stateful API, but this is the simpliest without
   * modifying jitcompiler.cpp much
   */
  void cInstrumentInterpreter(llvm::BasicBlock* bb);

  /**
   * set up a new context
   */
  void cPushContext(const InterpretedCallSignature& sig);

  /**
   * pop current context and use the previous context
   */
  void cPopContext();

  /**
   * assert to make sure contexts are properly poped
   */
  void cAssert() const;

  /**
   * shutdown background threads
   */
  void shutdown();

private:
  Profiler();
  Counters m_func_counts;
  Counters m_loop_counts;

  /**
   * interpreted counts
   */
  Counters m_itpr_counts;

  std::stack<Signature> m_contexts;

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
