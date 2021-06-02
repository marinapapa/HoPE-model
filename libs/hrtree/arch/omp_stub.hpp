// hrtree/arch/omp_stub.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ARCH_OMP_STUB_HPP_INCLUDED
#define HRTREE_ARCH_OMP_STUB_HPP_INCLUDED


#ifdef _WIN32   // Windows system specific
#include <windows.h>
#else          // Posix based system specific
#include <sys/time.h>
#endif


typedef struct 
{
  int owner;
  int count;
} omp_nest_lock_t;

typedef int omp_lock_t;

inline void omp_set_num_threads(int) {}
inline int omp_get_num_threads() { return 1; }
inline int omp_get_max_threads() { return 1; }
inline int omp_get_thread_num() { return 0; }
inline int omp_get_num_procs() { return 1; }
inline void omp_set_dynamic(int) {}
inline int omp_get_dynamic() { return 0; }
inline int omp_in_parallel() { return 0; }
inline void omp_set_nested() {}
inline int omp_get_nested() { return 0; }

enum {UNLOCKED = -1, INIT, LOCKED};
inline void omp_init_lock(omp_lock_t* lock) { *lock = UNLOCKED; }
inline void omp_destroy_lock(omp_lock_t* lock) { *lock = INIT; }
inline void omp_set_lock(omp_lock_t* lock) { *lock = LOCKED; }
inline void omp_unset_lock(omp_lock_t* lock) { *lock = UNLOCKED; }
inline int omp_test_lock(omp_lock_t* lock) { return *lock == UNLOCKED ? 1 : 0; }

enum {MASTER = 0};
inline void omp_init_nest_lock(omp_nest_lock_t* nLock) { nLock->owner = UNLOCKED; nLock->count = 0; }
inline void omp_destroy_nest_lock(omp_nest_lock_t* nLock) { nLock->owner = UNLOCKED; nLock->count = UNLOCKED; }

inline void omp_set_nest_lock(omp_nest_lock_t* nLock)
{
  if (nLock->owner == MASTER && nLock->count >= 1)
  {
    nLock->count = nLock->count + 1;
  } else {
    nLock->owner = MASTER;
    nLock->count = 1;
  }
}

inline void omp_unset_nest_lock(omp_nest_lock_t* nLock)
{
  if (nLock->owner == MASTER && nLock->count >= 1)
  {
    nLock->count = nLock->count - 1;
    if (nLock->count == 0)
    {
      nLock->owner = UNLOCKED;
    }
  }
}

inline int omp_test_nest_lock(omp_nest_lock_t* nLock) { return nLock->count; }
  
inline double omp_get_wtime()
{
#ifdef _WIN32
  LARGE_INTEGER now, freq;
  ::QueryPerformanceCounter(&now);
  ::QueryPerformanceFrequency(&freq);
  return (double)(now.QuadPart)/(double)freq.QuadPart;
#else
  timeval now;
  gettimeofday(&now, 0);
  return (double)(now.tv_sec + (1.0/1000000.0) * now.tv_usec);
#endif
}

inline double omp_get_wtick()
{
#ifdef _WIN32
  LARGE_INTEGER freq;
  ::QueryPerformanceFrequency(&freq);
  return 1.0/(double)freq.QuadPart;
#else
  return 1.0/1000000.0;   // 1us guessed
#endif
}

#endif

