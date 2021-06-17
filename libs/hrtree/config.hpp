// hrtree/config.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_CONFIG_DEFINED
#define HRTREE_CONFIG_DEFINED

#include <cstddef>
#include <utility>
#include <algorithm>


#ifndef HRTREE_NO_OPENMP
//  #define HRTREE_NO_OPNENMP
#endif

#if defined(_OPENMP) && !defined(HRTREE_NO_OPENMP)
  #include <omp.h>
  #ifndef HRTREE_OMP_MAX_THREADS
    #define HRTREE_OMP_MAX_THREADS 32
  #endif
  inline int hrtree_max_num_threads() { return std::min<int>(omp_get_max_threads(), HRTREE_OMP_MAX_THREADS); }
#else
  #include <hrtree/arch/omp_stub.hpp>
  #undef HRTREE_OMP_MAX_THREADS
  #define HRTREE_OMP_MAX_THREADS 1
  inline int hrtree_max_num_threads() { return 1; }
#endif

#define HRTREE_PARALLEL_PARTITION_BLOCK 256

#ifndef HRTREE_PARALLEL_ALIGNED_CONSTRUCT
// #define HRTREE_PARALLEL_ALIGNED_CONSTRUCT
#endif

#ifndef HRTREE_NO_VECTORIZER
//#define HRTREE_NO_VECTORIZER
#endif

#ifndef HRTREE_FORCE_UNALIGNED
//#define HRTREE_FORCE_UNALIGNED
#endif


#if !defined(HRTREE_NO_VECTORIZER) && defined(__SSE2__)
#define HRTREE_HAS_SSE2
#endif

#if !defined(HRTREE_NO_VECTORIZER) && defined(__SSE3__)
#define HRTREE_HAS_SSE3
#endif

#if !defined(HRTREE_NO_VECTORIZER) && defined(__AVX__)
#define HRTREE_HAS_AVX
#endif

#if !defined(HRTREE_NO_VECTORIZER) && defined(__AVX2__)
#define HRTREE_HAS_AVX2
#endif


#if defined (__GNUC__)
  #define HRTREE_ALIGN(n) __attribute__ ((aligned (n)))
  #define HRTREE_ALIGNOF(T) __alignof__(T)
#elif defined (_MSC_VER)
  #define HRTREE_ALIGN(n) __declspec(align(n))
  #define HRTREE_ALIGNOF(T) __alignof(T)
#else
  #error Please add equivalent of __attribute__((aligned(n))) and __alignof__(T) for your compiler
#endif

#ifndef HRTREE_ALIGN_CACHELINE
  #define HRTREE_ALIGN_CACHELINE HRTREE_ALIGN(64)
#endif
#define HRTREE_REQ_ALIGNMENT(T,A) (((HRTREE_ALIGNOF(T) % A) == 0))


#if (defined (_MSC_VER) && defined (_M_X64)) || (defined (__GNUC__) && defined (__LP64__))
  #define HRTREE_MALLOC_ALIGN 16
#else
  #define HRTREE_MALLOC_ALIGN 8
#endif


#endif
