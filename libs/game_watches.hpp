#ifndef GAME_WATCHES_HPP_INCLUDED
#define GAME_WATCHES_HPP_INCLUDED

// Hanno 2018


#include <chrono>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <type_traits>
#include <immintrin.h>


namespace game_watches {


  class spin_lock
  {
    mutable std::atomic_flag locked_ = ATOMIC_FLAG_INIT;

  public:
    void lock() const noexcept {
      while (locked_.test_and_set(std::memory_order_acquire)) {
        _mm_pause();   // spin but spin nicely
      }
    }
    void unlock() const noexcept {
      locked_.clear(std::memory_order_release);
    }
  };


  class no_lock
  {
  public:
    void lock() const noexcept {}
    void unlock() const noexcept {}
  };


  using default_clock_t = std::conditional_t<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock
  >;


  template <typename Clock = default_clock_t>
  class stop_watch
  {
  public:
    using clock = Clock;
    using clock_duration = typename Clock::duration;

    // starts the stopwatch if not already running.
    void start() noexcept {
      if (!is_running()) {
        accum_ -= clock::now().time_since_epoch();
      }
    }

    // stops the stopwatch if not already stopped.
    void stop() noexcept {
      if (is_running()) {
        accum_ += clock::now().time_since_epoch();
      }
    }

    // stops and resets the watch to zero.
    void reset() noexcept {
      accum_ = clock_duration::zero();
    }

    // combines reset() and start()
    void restart() noexcept {
      accum_ = -clock::now().time_since_epoch();
    }

    template <typename Duration>
    void sync_with(Duration d) {
      accum_ = std::chrono::duration_cast<clock_duration>(d);
    }

    bool is_running() const noexcept {
      return accum_ < clock_duration::zero();
    }

    // returns the accumulated duration the clock was running since the last reset.
    template <typename UserDuration = clock_duration>
    auto elapsed() const {
      if (is_running()) {
        return std::chrono::duration_cast<UserDuration>(accum_ + clock::now().time_since_epoch());
      }
      return std::chrono::duration_cast<UserDuration>(accum_);
    }

    // returns the accumulated duration the clock was running since the last reset in seconds.
    double elapsed_seconds() const {
      return elapsed<std::chrono::duration<double>>().count();
    }

  private:
    clock_duration accum_ = clock_duration::zero();
  };


  template <
    long long max_sleep_us = 10000,  // 10ms
    long long min_sleep_us = 100,    // 100us
    typename Clock = default_clock_t,
    typename Lock = spin_lock
  >
    class game_throttle
  {
  public:
    using clock = Clock;
    using duration = typename clock::duration;
    using time_point = typename clock::time_point;

    explicit game_throttle(double speedup = 1.0, double max_speedup = 1024.0) :
      speedup_((std::min)(speedup, max_speedup)),
      max_speedup_(max_speedup)
    {}

    ~game_throttle() {
      fast_forward(1);  // exit wait_for_sync
    }

    template <typename Duration>
    void tick(Duration t) const {
      {
        std::lock_guard<Lock> _(lock_);
        accum_ += std::chrono::duration_cast<duration>(t / speedup_);
      }
      // wait for sync
      for (;;) {
        auto d = sleep_duration();
        if (d < min_sleep()) break;
        std::this_thread::sleep_for(d);
      }
    }

    template <typename Duration, typename Fun>
    void tick(Duration t, Fun&& fun) const {
      {
        std::lock_guard<Lock> _(lock_);
        accum_ += std::chrono::duration_cast<duration>(t / speedup_);
      }
      // wait for sync
      for (;;) {
        auto d = sleep_duration();
        if (d < min_sleep()) break;
        std::this_thread::sleep_for(d);
        fun();
      }
    }

    template <typename Fun>
    double transform_speedup(Fun&& fun) const {
      std::lock_guard<Lock> _(lock_);
      speedup_ = (std::min)(fun(speedup_), max_speedup_);
      accum_ = clock::now();  // re-sync
      return speedup_;
    }

    template <typename Duration>
    auto diff() const {
      std::lock_guard<Lock> _(lock_);
      return std::chrono::duration_cast<Duration>(accum_ - clock::now());
    }

    template <typename Duration, typename TimePoint>
    auto diff(TimePoint T) const {
      std::lock_guard<Lock> _(lock_);
      return std::chrono::duration_cast<Duration>(accum_ - T);
    }

    bool is_paused() const noexcept {
      std::lock_guard<Lock> _(lock_);
      return paused_;
    }

    double speedup() const noexcept {
      std::lock_guard<Lock> _(lock_);
      return speedup_;
    }

    // returns current state
    bool toggle_pause() const noexcept {
      std::lock_guard<Lock> _(lock_);
      return paused_ = !paused_;
    }

    // returns current state
    bool pause(bool pause) const noexcept {
      std::lock_guard<Lock> _(lock_);
      return paused_ = pause;
    }

    // returns current state
    auto fast_forward(int frames) const noexcept {
      std::lock_guard<Lock> _(lock_);
      return fast_forward_ = frames;
    }

  private:
    static constexpr duration max_sleep() noexcept {
      return duration{ std::chrono::microseconds{ max_sleep_us } };
    }

    static constexpr duration min_sleep() noexcept {
      return duration{ std::chrono::microseconds{ min_sleep_us } };
    }

    duration sleep_duration() const {
      std::lock_guard<Lock> _(lock_);
      if (0 < fast_forward_) {
        if (0 == --fast_forward_) {
          accum_ = clock::now();  // re-sync
        }
        return duration{ 0 };
      }
      if (paused_) {
        accum_ = clock::now();  // re-sync
        return max_sleep();
      }
      if (speedup_ >= max_speedup_) {
        return duration{ 0 };
      }
      return (std::min)((accum_ - clock::now()) / 2, max_sleep());
    }

    mutable Lock lock_;
    mutable time_point accum_ = clock::now();
    mutable int fast_forward_ = 0;
    mutable bool paused_ = false;
    mutable double speedup_;
    const double max_speedup_;
  };

}

#endif
