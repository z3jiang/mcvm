DISCLAIMER
==========

This is a past version of McVM, with some parts that are work-in-progress and
not fully working.

BUILD IT
========

The easiest way to compile is to use the bootstrap.sh,
which will populate the /vendor directory with
LLVM and Boehm GC.

Some useful definitions are :
* MCVM_USE_JIT to compile the jit compiler (need LLVM) (recommended)
* MCVM_USE_LAPACKE
* MCVM_USE_CLAPACK
* MCVM_USE_DIMVECTOR
* MCVM_USE_PLOTTING

To run it, you will need a natlab.sh and a Natlab.jar in your PATH.

License
=========

License BSD - McGill University
