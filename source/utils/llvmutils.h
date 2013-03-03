/*
 * llvmutils.h
 *
 *  Created on: 2013-03-03
 *      Author: sam
 */

#ifndef LLVMUTILS_H_
#define LLVMUTILS_H_

#include <llvm/Type.h>
#include <llvm/LLVMContext.h>
#include <llvm/Constants.h>

namespace LLVMUtils
{

/**
 * must be called before any of the other methods
 */
extern void initialize(llvm::LLVMContext* context);

extern llvm::Type* VOID_PTR_TYPE;
//static llvm::Type* VOID_PTR_TYPE_IDIOM() ;


extern llvm::Type* getIntType(size_t sizeInBytes);

extern llvm::Constant* createPtrConst(
    const void* pointer, llvm::Type* valType = NULL);

} /* namespace LLVMUtils */
#endif /* LLVMUTILS_H_ */
