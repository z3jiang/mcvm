/*
 * llvmutils.cpp
 *
 *  Created on: 2013-03-03
 *      Author: sam
 */

#include <stdexcept>

#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Constants.h>

#include "llvmutils.h"
#include "../platform.h"

using namespace std;

llvm::LLVMContext* s_context = NULL;

// Void pointer type constant
// Calling a static LLVM Object can lead to bugs since the initialization order is undefined
// http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.15
// We use the Construct on First Use Idiom

//BUG
llvm::Type* LLVMUtils::VOID_PTR_TYPE = NULL;

// Correction
/*
llvm::Type* JITCompiler::VOID_PTR_TYPE() {
  static llvm::Type* llvm_type = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*s_Context));
  return llvm_type ;
}
*/

void LLVMUtils::initialize(llvm::LLVMContext* context)
{
  s_context = context;

  VOID_PTR_TYPE = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*s_context));
}

llvm::Type* LLVMUtils::getIntType(size_t sizeInBytes)
{
  switch (sizeInBytes)
  {
  // Return the appropriate integer type
  case 1: return llvm::Type::getInt8Ty(*s_context);
  case 2: return llvm::Type::getInt16Ty(*s_context);
  case 4: return llvm::Type::getInt32Ty(*s_context);
  case 8: return llvm::Type::getInt64Ty(*s_context);

  // Invalid integer sizes
  default:
    throw std::runtime_error("unexpected case");
  }
}

llvm::Constant* LLVMUtils::createPtrConst(
    const void* pointer, llvm::Type* valType)
{
  if (valType == NULL)
  {
    valType = llvm::Type::getInt8Ty(*s_context);
  }

  llvm::Type* intType = getIntType(PLATFORM_POINTER_SIZE);
  llvm::Constant* constInt = llvm::ConstantInt::get(intType, (int64)pointer);
  llvm::Constant* constPtr = llvm::ConstantExpr::getIntToPtr(
      constInt, llvm::PointerType::getUnqual(valType));

  return constPtr;
}
