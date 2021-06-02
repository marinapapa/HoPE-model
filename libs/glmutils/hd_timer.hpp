//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup timer High Resolution Timer
//!
//! Not related to anything else in this collection. However,
//! every (real-time) model needs a timer with a resolution
//! better than - say - 1ms. This class provides one for Windwos
//! and Posix-based platforms (UNIX Linux). Under Windows it
//! uses \c QueryPerformanceCounter(), under Posix \c gettimeofday().
//! The actual resolution is platform and system dependent but
//! supposed to be much higher than \c clock().
//!
//! \file hd_timer.hpp High Resolution Timer
//! for Windows ans Posix-based platforms (UNIX, Linux).
//! \ingroup timer


#ifndef glmutils_hd_timer_hpp
#define glmutils_hd_timer_hpp


#ifdef _WIN32   // Windows system specific
#include <windows.h>
#else          // Posix based system specific
#include <sys/time.h>
#endif


namespace glmutils {


  //! High Resolution Timer.
  class HDTimer
  {
  public:
    //! Constructor calls \ref restart()
    HDTimer()
    {
  #ifdef _WIN32
      LARGE_INTEGER freq;
      ::QueryPerformanceFrequency(&freq);
      rfreq_ = 1.0/(double)freq.QuadPart;
  #else
      gettimeofday(&start_, 0);
  #endif
      restart();
    }

    //! Restarts the timer
    void restart()
    {
  #ifdef _WIN32
      ::QueryPerformanceCounter(&start_);
  #else
      gettimeofday(&start_, 0);
  #endif
    }

    //! Returns elapsed time since \ref restart() in seconds
    double elapsed() const
    {
  #ifdef _WIN32
      LARGE_INTEGER now;
      ::QueryPerformanceCounter(&now);
      return (double)(now.QuadPart - start_.QuadPart) * rfreq_;
  #else
      timeval now;
      gettimeofday(&now, 0);
      return (double)((now.tv_sec - start_.tv_sec) + (1.0/1000000.0) * (now.tv_usec - start_.tv_usec));
  #endif
    }

  private:
  #ifdef _WIN32
    double      rfreq_;    // 1/(ticks per second)
    LARGE_INTEGER  start_;
  #else
    timeval      start_;
  #endif
  };


}   // namespace glmutils



#endif
