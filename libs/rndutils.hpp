#ifndef RNDUTILS_HPP_INCLUDED
#define RNDUTILS_HPP_INCLUDED

//
// - xorshift random number generators.
// - foolproof seeding.
// - faster generate_canonical template.
// - some 'missing' distributions.
//
// For best performance, include rndutils.hpp before or instead of <random>
//
// Hanno Hildenbrandt 2015-2018

#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <random>
#include <chrono>
#include <thread>
#include <memory>
#include <array>
#include <functional>
#include <initializer_list>
#include <algorithm>
#include <numeric>
#include <istream>
#include <iosfwd>


// clang-format off
#if !defined(RNDUTILS_DEFAULT_ENGINE)
  // Besides cryptography hardly any reason to use something else.
  #define RNDUTILS_DEFAULT_ENGINE rndutils::xorshift128
#endif


#if !defined(RNDUTILS_DISABLE_FAST_STD_GENERATE_CANONICAL)
  #define RNDUTILS_ENABLE_FAST_STD_GENERATE_CANONICAL
#endif


#if defined(__has_builtin)
  // CLang
  #if !defined(RNDUTILS_CPU_TIMESTAMP_COUNTER)
    #if __has_builtin(__builtin_readcyclecounter)
      #if !defined(__c2__)  // not yet implemented in CLANG/C2
        #define RNDUTILS_CPU_TIMESTAMP_COUNTER __builtin_readcyclecounter()
      #endif
    #endif  
  #endif
  #if __has_builtin(__builtin_clz)
    #define RNDUTILS_MSB_U32(x) (x ? 32 - __builtin_clz(x) : 0)
  #endif
  #if __has_builtin(__builtin_clzll)
    #define RNDUTILS_MSB_U64(x) (x ? 64 - __builtin_clzll(x) : 0)
  #endif
  #if __has_builtin(__builtin_popcount)
    #define RNDUTILS_POP_COUNT(x) __builtin_popcount(x)
  #endif
  #if __has_builtin(__builtin_popcountl)
    #define RNDUTILS_POP_COUNT64(x) __builtin_popcountl(x)
  #endif
#elif defined(_M_X64) || defined(_M_IX86) || defined(__i386__) || defined(__amd64__)
  #if defined(__GNUC__)
    #include <x86intrin.h>
    #define RNDUTILS_MSB_U32(x) (x ? 32 - __builtin_clz(x) : 0)
    #define RNDUTILS_POP_COUNT(x) __builtin_popcount(x)
    #define RNDUTILS_POP_COUNT64(x) __builtin_popcountl(x)
    #if defined(__amd64__)
      #define RNDUTILS_MSB_U64(x) (x ? 64 - __builtin_clzll(x) : 0)
    #endif
  #elif defined(_MSC_VER)
    #include <intrin.h>
    #define RNDUTILS_MSB_U32(x) static_cast<int>(x ? 32u - _lzcnt_u32(x) : 0u)
    #define RNDUTILS_POP_COUNT(x) __popcnt(x)
    #define RNDUTILS_POP_COUNT64(x) __popcnt64(x)
    #if defined(_M_X64)
      #define RNDUTILS_MSB_U64(x) static_cast<int>(x ? 64ull - _lzcnt_u64(x) : 0ull)
    #endif
  #endif
  #if !defined(RNDUTILS_CPU_TIMESTAMP_COUNTER)
    #define RNDUTILS_CPU_TIMESTAMP_COUNTER __rdtsc()
  #endif
#endif
  // give up
  #if !defined(RNDUTILS_CPU_TIMESTAMP_COUNTER)
  #define RNDUTILS_CPU_TIMESTAMP_COUNTER 5489u
#endif


#if !defined(RNDUTILS_MSB_U32)
  #define RNDUTILS_MSB_U32(x) (x ? debruijn_bitscanreverse::bsr(x) + 1 : 0)
#endif
#if !defined(RNDUTILS_MSB_U64)
  #define RNDUTILS_MSB_U64(x) (x >= (uint64_t(1) << 31) ? 31 + RNDUTILS_MSB_U32(uint32_t(x >> 31)) : RNDUTILS_MSB_U32(uint32_t(x)))
#endif
#if !defined(RNDUTILS_POP_COUNT)
  #define RNDUTILS_POP_COUNT(x) popcount(x)
#endif
#if !defined(RNDUTILS_POP_COUNT64)
  #define RNDUTILS_POP_COUNT64(x) (popcount(x & 0x00000000FFFFFFFF) + popcount(x >> 31))
#endif
// clang format on


namespace rndutils {


namespace detail {


  // De Bruijn multiplication
  struct debruijn_bitscanreverse
  {
    static constexpr int index32[32] = {
      0, 9, 1, 10, 13, 21, 2, 29,
      11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7,
      19, 27, 23, 6, 26, 5, 4, 31};

    static constexpr int bsr(uint32_t x) noexcept
    {
      x |= x >> 1;
      x |= x >> 2;
      x |= x >> 4;
      x |= x >> 8;
      x |= x >> 16;
      return index32[(x * 0x07C4ACDD) >> 27];
    }
  };


  inline constexpr int popcount(uint32_t x) noexcept
  {
    // bit count, Hacker's delight
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
  }


  inline int popcount(uint64_t x) noexcept
  {
    return static_cast<int>(RNDUTILS_POP_COUNT64(x));
  }
    
    
  inline int msb_uint(uint8_t x) noexcept
  {
    return RNDUTILS_MSB_U32(uint32_t{ x });
  }


  inline int msb_uint(uint16_t x) noexcept
  {
    return RNDUTILS_MSB_U32(uint32_t{ x });
  }


  inline int msb_uint(uint32_t x) noexcept
  {
    return RNDUTILS_MSB_U32(x);
  }


  inline int msb_uint(uint64_t x) noexcept
  {
    return RNDUTILS_MSB_U64(x);
  }


  template <bool Test, typename T>
  using enable_if_t = typename std::enable_if<Test, T>::type;


  template <int I, uint64_t Val>
  struct range_digits_impl : std::integral_constant<int, (Val > 1) ? range_digits_impl<I + 1, (Val >> 1)>::value : I>
  {
  };


  template <int I>
  struct range_digits_impl<I, 1> : std::integral_constant<int, I>
  {
  };


  template <uint64_t Range>
  struct range_digits : std::integral_constant<int, range_digits_impl<0, Range + 1>::value>
  {
  };


  template <>
  struct range_digits<std::numeric_limits<uint64_t>::max()> : std::integral_constant<int, std::numeric_limits<uint64_t>::digits>
  {
  };


  template <typename URNG>
  struct urng_range
  {
    using result_type = typename URNG::result_type;
    static_assert(std::is_unsigned<result_type>::value, "illegal random number engine result_type in urng_range");
    static constexpr result_type range = URNG::max() - URNG::min();
    static constexpr int digits = range_digits<range>::value;
  };


  template <typename Real, size_t Bits, typename URNG>
  struct generate_canonical_constants
  {
    using real_t = Real;
    using urng_t = URNG;
    using range_t = urng_range<URNG>;
    //using result_t = URNG::result_type;
    static constexpr auto digits = static_cast<size_t>(std::numeric_limits<Real>::digits);
    static constexpr auto bits = digits < Bits ? digits : Bits;
    static constexpr auto samples = (bits - 1u + range_t::digits) / range_t::digits;
    static constexpr auto range = static_cast<Real>(range_t::range);
    static constexpr auto gmin = static_cast<Real>(URNG::min());
  };


  template <typename GCB>
  inline auto do_generate_canonical(typename GCB::urng_t& rgen) noexcept
    -> enable_if_t<(GCB::samples == 1), typename GCB::real_t>
  {
    const auto rx = static_cast<typename GCB::real_t>(1) / (GCB::range + static_cast<typename GCB::real_t>(1));
    const auto s = static_cast<typename GCB::real_t>(rgen()) - GCB::gmin;
    return s * rx;
  }


  template <typename GCB>
  inline auto do_generate_canonical(typename GCB::urng_t& rgen) noexcept
    -> enable_if_t<(GCB::samples == 2), typename GCB::real_t>
  {
    const auto rx = GCB::range + static_cast<typename GCB::real_t>(1);
    const auto s1 = static_cast<typename GCB::real_t>(rgen()) - GCB::gmin;
    const auto s2 = (static_cast<typename GCB::real_t>(rgen()) - GCB::gmin) * rx;
    return (s1 + s2) / (rx * rx);
  }


  template <typename GCB>
  inline auto do_generate_canonical(typename GCB::urng_t& rgen) noexcept
    -> enable_if_t<(GCB::samples > 2), typename GCB::real_t>
  {
    const auto rx = GCB::range + static_cast<typename GCB::real_t>(1);
    auto res = static_cast<typename GCB::real_t>(0);
    auto factor = static_cast<typename GCB::real_t>(1);

    for (int s = 0; s < GCB::samples; ++s) {
      res += (static_cast<typename GCB::real_t>(rgen()) - GCB::gmin) * factor;
      factor *= rx;
    }
    return (res / factor);
  }


  // returns canonical random number in [0,1)
  // fast version for URNG returning unsigned result.
  template <typename Real, size_t Bits, typename URNG>
  inline auto generate_canonical(URNG& reng) noexcept
    -> enable_if_t<std::is_unsigned<typename URNG::result_type>::value, Real>
  {
    using GCB = generate_canonical_constants<Real, Bits, URNG>;
    return do_generate_canonical<GCB>(reng);
  }


  // returns canonical random number in [0,1)
  // fall-back version for RNG returning non-unsigned result.
  template <typename Real, size_t Bits, typename RNG>
  inline auto generate_canonical(RNG& reng)
    -> enable_if_t<!std::is_unsigned<typename RNG::result_type>::value, Real>
  {
    return std::generate_canonical<Real, Bits, RNG>(reng);
  }


  // returns high-entropy 512 bit array for seed sequence
  inline auto make_high_entropy_seed_array() -> std::array<uint64_t, 8>
  {
    // if supported, quite good: the CPU time-stamp
    const auto e0 = static_cast<uint64_t>(RNDUTILS_CPU_TIMESTAMP_COUNTER);
    // the classic: time, advertised with nano-second resolution.
    const auto e1 = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    // with some luck, this is non-deterministic: TRNG
    const auto e2 = static_cast<uint64_t>(std::random_device{}());
    // different between invocations from different threads within one app: thread-id
    const auto tid = std::this_thread::get_id();
    const uint64_t e3{ std::hash<std::remove_const_t<decltype(tid)>>()(tid) };
    // different between installations: compile time macros
    const auto e4 = static_cast<uint64_t>(std::hash<const char*>()(__DATE__ __TIME__ __FILE__));
    // different between platforms: typeid
    const auto e5 = static_cast<uint64_t>(typeid(make_high_entropy_seed_array).hash_code());
    // likely different between runs, invocations and platforms: address of local
    const auto e6 = reinterpret_cast<uint64_t>(&e0);
	return {{ e0, e1, e2, e3, e4, e5, e6 }};
  }


  // returns low-entropy 512 bit array for seed sequence
  // based on std::chrono::high_resolution_clock.
  inline auto make_low_entropy_seed_array() noexcept -> std::array<uint64_t, 8>
  {
	  // the classic: time, advertised with nano-second resolution.
	  const auto e1 = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	  // different between invocations from different threads within one app: thread-id
	  const auto tid = std::this_thread::get_id();
	  const uint64_t e2{ std::hash<std::remove_const_t<decltype(tid)>>()(tid) };
	  return std::array<uint64_t, 8>{{
        e1, e2,
        0x000000003c10b019, 0x2bf820b4dd7c1a8a,
        0x9901cf90a40883da, 0x5a3686b2e1de6e51,
        0x000000cc0494d228, 0x000000cc04b66740
    }};
  }
}


//
// Random number generators from the xorshift family
//


// xorshift128+ random number generator.
// Very fast, small state, 64bit result, good quality.
class xorshift128
{
public:
  using result_type = uint64_t;
  using engine_type = xorshift128;

  static constexpr uint64_t default_seed = static_cast<uint64_t>(0xccf05c43b8137c1f);
  static constexpr uint64_t (min)() { return 0; };
  static constexpr uint64_t (max)() { return -1; };

  template <typename T>
  static T canonical(uint64_t rnd)
  {
    using GCB = detail::generate_canonical_constants<T, 64, xorshift128>;
    const auto rx = static_cast<typename GCB::real_t>(1) / (GCB::range + static_cast<typename GCB::real_t>(1));
    const auto s = static_cast<typename GCB::real_t>(rnd) - GCB::gmin;
    return s * rx;
  }

  explicit xorshift128(uint64_t val = default_seed) noexcept
  {
    seed(val);
  }

  explicit xorshift128(std::seed_seq& sseq) noexcept
  {
    seed(sseq);
  }

  void seed(uint64_t val = default_seed) noexcept
  {
    std::seed_seq sseq{val};
    seed(sseq);
  }

  void seed(std::seed_seq& sseq) noexcept
  {
    auto* p = reinterpret_cast<uint32_t*>(state_.data());
    sseq.generate(p, p + 2 * state_.size());
  }

  uint64_t operator()(void) noexcept
  {
    uint64_t s0 = state_[0];
    const uint64_t s1 = state_[1];
    state_[0] = s1;
    s0 ^= s0 << 23u;
    s0 ^= s0 >> 17u;
    s0 ^= s1 ^ (s1 >> 26u);
    state_[1] = s0;
    return s0 + s1;
  }

  void discard(unsigned long long z) noexcept
  {
    for (unsigned long long i = 0; i < z; ++i) this->operator()();
  }

  friend bool operator==(engine_type const& lhs, engine_type const& rhs) noexcept
  {
    return lhs.state_[0] == rhs.state_[0] && lhs.state_[1] == rhs.state_[1];
  }

  friend bool operator!=(engine_type const& lhs, engine_type const& rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, engine_type const& reng) -> std::basic_ostream<CharT, Traits>&
  {
    for (auto x : reng.state_) os << x << ' ';
    return os;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, engine_type const& reng) -> std::basic_istream<CharT, Traits>&
  {
    for (auto& x : reng.state_) is >> x >> std::ws;
    return is;
  }

private:
  std::array<uint64_t, 2> state_;
};


// xorshift1024* random number generator.
// Fast, small state, 64bit result, excellent quality.
class xorshift1024
{
public:
  using result_type = uint64_t;
  using engine_type = xorshift1024;
  static constexpr uint64_t default_seed = static_cast<uint64_t>(0xdf7e67f833c8551b);
  static constexpr uint64_t(min)() { return static_cast<uint64_t>(0); };
  static constexpr uint64_t(max)() { return static_cast<uint64_t>(-1); };

  explicit xorshift1024(uint64_t val = default_seed) noexcept
  {
    seed(val);
  }

  explicit xorshift1024(std::seed_seq& sseq) noexcept
  {
    seed(sseq);
  }

  void seed(uint64_t val = default_seed) noexcept
  {
    std::seed_seq sseq{val};
    seed(sseq);
  }

  void seed(std::seed_seq& sseq) noexcept
  {
    auto* p = reinterpret_cast<uint32_t*>(state_.data());
    sseq.generate(p, p + 2 * state_.size());
    pivot_ = 0;
  }

  uint64_t operator()(void) noexcept
  {
    uint64_t s0 = state_[pivot_];
    uint64_t s1 = state_[pivot_ = (pivot_ + 1) & 15u];
    s1 ^= s1 << 31u;
    s1 ^= s1 >> 11u;
    s0 ^= s0 >> 30u;
    return (state_[pivot_] = s0 ^ s1) * static_cast<uint64_t>(0x106689d45497fdb5);
  }

  void discard(unsigned long long z) noexcept
  {
    for (unsigned long long i = 0; i < z; ++i) this->operator()();
  }

  friend bool operator==(engine_type const& lhs, engine_type const& rhs) noexcept
  {
    return lhs.state_ == rhs.state_;
  }

  friend bool operator!=(engine_type const& lhs, engine_type const& rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, engine_type const& reng) -> std::basic_ostream<CharT, Traits>&
  {
    for (auto x : reng.state_) os << x << ' ';
    return os;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, engine_type const& reng) -> std::basic_istream<CharT, Traits>&
  {
    for (auto& x : reng.state_) is >> std::ws >> x;
    return is;
  }

private:
  std::array<uint64_t, 16> state_;
  int pivot_;
};


//
// Seeding support
//


using default_engine = RNDUTILS_DEFAULT_ENGINE;


// random number generator from high-entropy seed sequence
template <typename URNG = default_engine>
inline auto make_random_engine() -> URNG
{
  const auto seed_array = detail::make_high_entropy_seed_array();
  std::seed_seq sseq(seed_array.cbegin(), seed_array.cend());
  return URNG(sseq);
}


// random number generator from low-entropy seed sequence
template <typename URNG = default_engine>
inline auto make_random_engine_low_entropy() -> URNG
{
  auto seed_array = detail::make_low_entropy_seed_array();
  std::seed_seq sseq(seed_array.cbegin(), seed_array.cend());
  return URNG(sseq);
}


// random number generator from single seed value
template <typename URNG = default_engine>
inline auto make_random_engine(typename URNG::result_type seed) -> URNG
{
  const auto seed_array = std::array<uint64_t, 8>{{
      0x000021ac9bcc0a2e ^ static_cast<uint64_t>(seed),
      0x000041f2ea94a095,
      0x000000003c10b019, 0x2bf820b4dd7c1a8a,
      0x9901cf90a40883da, 0x5a3686b2e1de6e51,
      0x000000cc0494d228, 0x000000cc04b66740}};
  std::seed_seq sseq(seed_array.cbegin(), seed_array.cend());
  return URNG(sseq);
}


//
// Extra random distributions
//


// returns random number in [0,1)
// alias to generate_canonical
template <typename Real = double, typename URNG>
inline Real uniform01(URNG& reng)
{
  return detail::generate_canonical<Real, size_t(-1), URNG>(reng);
};


// The simplest one. Just missing in <random>
template <typename T = double>
class const_distribution
{
public:
  using distribution_type = const_distribution;
  using result_type = T;
  using param_type = T;

  constexpr T(min)() const { return val_; }
  constexpr T(max)() const { return val_; }

  const_distribution() 
  {
  }

  explicit const_distribution(const param_type& val)
  : val_(val)
  {
  }

  param_type param() const { return {}; }
  void param(const param_type& val) { val_ = val; }
  void reset() {}

  template <typename URNG>
  T operator()(URNG&) const { return val_; }

  template <typename URNG>
  T operator()(URNG&, param_type const& val) const { return val; }

  friend bool operator==(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.val_ == rhs.val_;
  }

  friend bool operator!=(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.val_ != rhs.val_;
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, distribution_type const& dist) -> std::basic_ostream<CharT, Traits>&
  {
    os << dist.val_;
    return os;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, distribution_type& dist) -> std::basic_istream<CharT, Traits>&
  {
    is >> dist.val_;
    return is;
  }

private:
  T val_;
};


template <typename Real = double>
class uniform01_distribution
{
public:
  static_assert(std::is_floating_point<Real>::value, "invalid template argument for uniform01_distribution");

  using distribution_type = uniform01_distribution;
  using result_type = Real;
  struct param_type
  {
    using distribution_type = uniform01_distribution;
  };

  static constexpr Real(min)() { return static_cast<Real>(0); }
  static constexpr Real(max)() { return static_cast<Real>(1); }

  uniform01_distribution() {}
  explicit uniform01_distribution(const param_type&) {}

  param_type param() const
  {
    return {};
  }
  void param(const param_type&) {}
  void reset() {}

  template <typename URNG>
  Real operator()(URNG& reng) const
  {
    return uniform01<Real>(reng);
  }

  template <typename URNG>
  Real operator()(URNG& reng, param_type const&) const
  {
    return uniform01<Real>(reng);
  }

  friend bool operator==(distribution_type const&, distribution_type const&)
  {
    return true;
  }
  friend bool operator!=(distribution_type const&, distribution_type const&)
  {
    return false;
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, distribution_type const&) -> std::basic_ostream<CharT, Traits>&
  {
    return os;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, distribution_type&) -> std::basic_istream<CharT, Traits>&
  {
    return is;
  }
};


// Replacement for std::uniform_int_distribution.
// Somewhat ill-named - uses the one bit for faster evaluation.
template <typename IntT = int>
class uniform_signed_distribution
{
  using UIntT = typename std::make_unsigned<IntT>::type;

public:
  static_assert(std::is_signed<IntT>::value && std::is_integral<IntT>::value, 
                "invalid template argument for uniform_signed_distribution");

  using result_type = IntT;
  using distribution_type = uniform_signed_distribution<IntT>;

  struct param_type
  {
    using distribution_type = uniform_signed_distribution<IntT>;
    UIntT range;
    IntT lo;
    int digits;
  };

  constexpr IntT(min)() const { return p_.lo; }
  constexpr IntT(max)() const { return p_.lo + p_.range; }

  uniform_signed_distribution(IntT Min, IntT Max)
  {
    using std::abs;
    p_.range = static_cast<UIntT>(Max - Min);
    p_.lo = Min;
    p_.digits = detail::msb_uint(UIntT(p_.range + 1));
  }


  template <typename URNG>
  IntT operator()(URNG& reng) const
  {
    static_assert(std::is_unsigned<typename URNG::result_type>::value, 
                  "invalid template argument for uniform_signed_distribution::operator()");
    // dispatch by digits-requirement
    return eval(reng, p_, std::integral_constant<bool, rndutils::detail::urng_range<URNG>::digits >= std::numeric_limits<IntT>::digits>());
  }

  template <typename URNG>
  IntT operator()(URNG& reng, param_type const& p) const
  {
    static_assert(std::is_unsigned<typename URNG::result_type>::value, 
                  "invalid template argument for uniform_signed_distribution::operator()");
    // dispatch by digits-requirement
    return eval(reng, p, std::integral_constant<bool, rndutils::detail::urng_range<URNG>::digits >= std::numeric_limits<IntT>::digits>());
  }

  param_type param() const
  {
    return p_;
  }

  void param(const param_type& Param)
  {
    p_ = Param;
  }

  void reset() {}

  friend bool operator==(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.p_ == rhs.p_;
  }
  friend bool operator!=(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.p_ != rhs.p_;
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, distribution_type const& dist) -> std::basic_ostream<CharT, Traits>&
  {
    os << dist.p_.range << ' ' << dist.p_.lo << ' ' << dist.p_.digits;
    return os;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, distribution_type& dist) -> std::basic_istream<CharT, Traits>&
  {
    param_type p;
    is >> std::ws >> p.range >> std::ws >> p.lo >> std::ws >> p.digits;
    if (is) dist.p_ = p;
    return is;
  }

private:
  template <typename URNG>
  IntT eval(URNG& reng, param_type const& p, std::true_type) const
  { // sufficient digits in URNG
    const int shift = rndutils::detail::urng_range<URNG>::digits - p.digits;
    UIntT x;
    do
    {
      x = static_cast<UIntT>(reng() >> shift);
    } while (x > p.range);
    return static_cast<IntT>(x) + p.lo;
  }

  template <typename URNG>
  IntT eval(URNG& reng, param_type const& p, std::false_type) const
  { // insufficient digits in URNG
    using urng_range = rndutils::detail::urng_range<URNG>;
    const auto mask = ~(static_cast<UIntT>(-1) << p.digits);
    UIntT x;
    do
    {
      x = static_cast<UIntT>(0);
      for (int b = 0; b < p.digits_; b += urng_range::digits) {
        x <<= urng_range::digits;
        x ^= static_cast<UIntT>(reng());
      }
      x &= mask;
    } while (x > p.range);
    return static_cast<IntT>(x) + p.lo;
  }

  param_type p_;
};


// binary_distribution 0|1.
// Very fast if used several times per instantiation.
// Also missing in <random>.
template <typename ResT = bool>
class binary_distribution
{
  using bits_type = uintmax_t;
  static constexpr int msb = std::numeric_limits<bits_type>::digits - 1;

public:
  using distribution_type = binary_distribution;
  using result_type = ResT;
  struct param_type
  {
    using distribution_type = binary_distribution;
  };

  static constexpr result_type(min)() { return false; }
  static constexpr result_type(max)() { return true; }

  binary_distribution() : 
    bits_(1)  // set stop-bit
  {
  }

  explicit binary_distribution(const param_type& param)
  { // construct from parameter package
    this->param(param);
  }

  param_type param() const
  { // return parameter package
    return {};
  }

  void param(const param_type& /*param*/)
  { // set parameter package
  }

  void reset()
  { // reset, set stop-bit
    bits_ = static_cast<bits_type>(1);
  }

  template <typename URNG>
  result_type operator()(URNG& reng)
  {
    using urng_range = detail::urng_range<URNG>;
    if (bits_ == static_cast<bits_type>(1)) {
      bits_ = static_cast<bits_type>(reng());
      auto stopbit = (urng_range::digits + 1) < msb ? (urng_range::digits + 1) : msb;
      bits_ |= static_cast<bits_type>(1) << static_cast<unsigned>(stopbit);
    }
    auto res = static_cast<result_type>(bits_ & 1u);
    bits_ >>= 1u;
    return res;
  }

  template <typename URNG>
  result_type operator()(URNG& reng, param_type const& /*param*/)
  {
    return this->operator()(reng);
  }

  friend bool operator==(distribution_type const& lhs, distribution_type const& rhs)
  {
    return true;
  }
  friend bool operator!=(distribution_type const& lhs, distribution_type const& rhs)
  {
    return false;
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, distribution_type const& dist) -> std::basic_ostream<CharT, Traits>&
  { // output streaming
    return os << dist.bits_;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, distribution_type& dist) -> std::basic_istream<CharT, Traits>&
  { // input streaming
    bits_type bits;
    is >> std::ws >> bits;
    if (is) dist.bits_ = bits;
    return is;
  }

private:
  bits_type bits_;
};


// binomial distribution with p=0.5.
// direct pop-count sampling, potentially very fast for small n, up to n~10000. 
template <
  typename ResT = int
>
class binomial50_small_distribution
{
public:
  using distribution_type = binomial50_small_distribution;
  using result_type = ResT;
  using param_type = result_type;

  static constexpr result_type(min)() { return result_type{ 0 }; }
  static constexpr result_type(max)() { return result_type{ std::numeric_limits<result_type>::max() }; }

  explicit binomial50_small_distribution(result_type n) : 
    param_{ n }
  {
  }

  param_type param() const
  { // return parameter package
    return param_;
  }

  void param(param_type param)
  { // set parameter package
    param_ = param;
  }

  void reset()
  {
  }

  template <typename URNG>
  result_type operator()(URNG& reng) const
  {
    return eval(param_, reng);
  }

  template <typename URNG>
  result_type operator()(URNG& reng, param_type param) const
  {
    return eval(param, reng);
  }

  friend bool operator==(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.param_ == rhs.param_;
  }
  friend bool operator!=(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.param_ != rhs.param_;
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, distribution_type const& dist) -> std::basic_ostream<CharT, Traits>&
  { // output streaming
    return os << dist.param_;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, distribution_type& dist) -> std::basic_istream<CharT, Traits>&
  { // input streaming
    is >> std::ws >> dist.param_;
    return is;
  }

private:
  template <typename URNG>
  result_type eval(param_type n, URNG& reng) const
  {
    if (n == 0) return 0;
    using ur = detail::urng_range<URNG>;
    int64_t s = 0;
    for (; n > ur::digits; n -= ur::digits) {
      s += static_cast<int64_t>(detail::popcount(reng()));
    }
    const auto mask = typename ur::result_type(-1) >> (ur::digits - n);
    return static_cast<result_type>(s + static_cast<int64_t>(detail::popcount(reng() & mask)));
  }

  param_type param_;
};


// Policy classes for the mutable_discrete_distribution below.

// throw if all zero
struct all_zero_policy_throw {};


// assert if all zero (this is what std::discrete_distribution does)
struct all_zero_policy_assert {};


// revert to uniform distribution if all zero
struct all_zero_policy_uni {};


// Replacement for std::discrete_distribution.
// Use it if your distribution is non-const and you want
// to avoid the extra memory allocation to produce a fresh
// std::discrete_distribution or you are not satisfied 
// with the behavior of std::discrete_distribution in the
// case that all elements are zero.
template <typename IntType = int, 
          typename AllZeroPolicy = all_zero_policy_throw, 
          typename ParamType = std::vector<double>
>
class mutable_discrete_distribution
{
public:
  using distribution_type = mutable_discrete_distribution;
  using result_type = IntType;
  using param_type = ParamType;
  using weight_type = typename param_type::value_type;

  static_assert(std::is_integral<result_type>::value, 
                "Invalid template argument for mutable_discrete_distribution");
  static_assert(std::is_floating_point<weight_type>::value, 
                "Invalid template argument for mutable_discrete_distribution");

  result_type min() const
  {
    return result_type(0);
  }
  result_type max() const
  {
    return static_cast<result_type>(cdf_.size() - 1);
  }


  mutable_discrete_distribution()
  : cdf_(1, result_type(0))
  { // default ctor
  }

  explicit mutable_discrete_distribution(param_type const& param)
  {
    mutate(param.cbegin(), param.cend());
  }

  template <typename InIt>
  mutable_discrete_distribution(InIt first, InIt last)
  {
    mutate(first, last);
  }

  void param(param_type const& param)
  {
    mutate(param.cbegin(), param.cend());
  }

  const param_type& cdf() const { return cdf_; }

  param_type param() const
  {
    param_type pval(cdf_);
    std::adjacent_difference(cdf_.cbegin(), cdf_.cend(), pval.begin());
    return pval;
  }

  std::vector<double> probabilities() const
  {
    auto sum = static_cast<double>(cdf_.back());
    if (sum == 0.0) return std::vector<double>(1, 1.0);
    auto pval = param();
    std::vector<double> pdf;
    for (auto p : pval) pdf.push_back(static_cast<double>(p) / sum);
    return pdf;
  }

  void reset()
  {
  }

  template <typename Reng>
  result_type operator()(Reng& reng) const
  {
    auto p = cdf_.back() * uniform01<weight_type>(reng);
    return static_cast<result_type>(std::distance(cdf_.cbegin(), std::lower_bound(cdf_.cbegin(), cdf_.cend(), p)));
  }

  template <typename Reng>
  result_type operator()(Reng& reng, param_type const& param) const
  {
    return distribution_type(param)(reng);
  }

  template <typename InIt>
  void mutate(InIt first, InIt last)
  {
    mutate_transform_partial(first, last, 0, [](weight_type w) {
      return w;
    });
  }

  template <typename W>
  void mutate(W const& w)
  {
    mutate_transform_partial(w.cbegin(), w.cend(), 0, [](weight_type w) {
      return w;
    });
  }

  template <typename InIt>
  void mutate_partial(InIt first, InIt last, size_t ofs)
  {
    mutate_transform_partial(first, last, ofs, [](weight_type w) {
      return w;
    });
  }

  template <typename W>
  void mutate_partial(W const& w, size_t ofs)
  {
    mutate_transform_partial(w.cbegin(), w.cend(), ofs, [](weight_type w) {
      return w;
    });
  }

  template <typename InIt, typename Fun>
  void mutate_transform(InIt first, InIt last, Fun fun)
  {
    mutate_transform_n(first, std::distance(first, last), fun);
  }

  template <typename InIt, typename Fun>
  void mutate_transform_n(InIt first, size_t N, Fun fun)
  {
    mutate_transform_partial_n(first, N, 0, fun);
  }

  template <typename W, typename Fun>
  void mutate_transform(W const& w, Fun fun)
  {
    mutate_transform_partial(w.cbegin(), w.cend(), 0, fun);
  }

  template <typename InIt, typename Fun>
  void mutate_transform_partial(InIt first, InIt last, size_t ofs, Fun fun)
  {
    const auto N = static_cast<size_t>(std::distance(first, last));
    mutate_transform_partial_n(first, N, ofs, fun);
  }

  template <typename InIt, typename Fun>
  void mutate_transform_partial_n(InIt first, size_t N, size_t ofs, Fun fun)
  {
    if (N == 0) {
      // degenerated distribution is fine but special case
      cdf_.assign(1, weight_type(1));
      return;
    }
    assert(ofs < N && "Invalid offset for mutable_partial");
    cdf_.resize(N);
    weight_type sum(ofs > 0 ? cdf_[ofs - 1] : weight_type(0));
    for (size_t i = ofs; i < N; ++i, ++first) {
      auto x = static_cast<weight_type>(fun(*first));
      assert(x >= weight_type(0) && "Negative weight in mutable_discrete_distribution");
      cdf_[i] = sum += x;
    }
    apply_all_zero_policy(sum, AllZeroPolicy{});
  }

  friend bool operator==(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.cdf_ == rhs.cdf_;
  }

  friend bool operator!=(distribution_type const& lhs, distribution_type const& rhs)
  {
    return lhs.cdf_ != rhs.cdf_;
  }

  template <class CharT, class Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& os, distribution_type const& dist) -> std::basic_ostream<CharT, Traits>&
  { // output streaming
    os << dist.cdf_.size();
    for (auto p : dist.cdf_) os << ' ' << p;
    return os;
  }

  template <class CharT, class Traits>
  friend auto operator>>(std::basic_istream<CharT, Traits>& is, distribution_type& dist) -> std::basic_istream<CharT, Traits>&
  { // input streaming
    size_t size;
    is >> size;
    param_type cdf;
    for (size_t i = 0; i < size; ++i) {
      weight_type w;
      is >> std::ws >> w;
      cdf.push_back(w);
    }
    if (is) dist.cdf_ = std::move(cdf);
    return is;
  }

private:
  void apply_all_zero_policy(weight_type sum, all_zero_policy_throw)
  { // throw
    if (sum <= weight_type(0)) {
      throw std::invalid_argument("Invalid weight-vector for mutable_discrete_distribution");
    }
  }

  void apply_all_zero_policy(weight_type sum, all_zero_policy_assert)
  { // assert
    assert(sum > weight_type(0) && "Invalid weight-vector for mutable_discrete_distribution");
  }

  void apply_all_zero_policy(weight_type sum, all_zero_policy_uni)
  { // degenerate to uniform distribution
    if (sum <= weight_type(0)) {
      auto s = weight_type(1);
      for (auto& x : cdf_) {
        x = s++;
      }
    }
  }

  param_type cdf_;
};


//
// Algorithms
//


template <typename OIt, typename URNG>
void generate_uniform_n(size_t n, URNG& reng, OIt out)
{
  for (; n > 4; n -= 4) {
    auto a0 = uniform01(reng);
    auto a1 = uniform01(reng);
    auto a2 = uniform01(reng);
    auto a3 = uniform01(reng);
    *out++ = a0;
    *out++ = a1;
    *out++ = a2;
    *out++ = a3;
  }
  for (; n > 0; --n) {
    *out++ = uniform01(reng);
  }
}


template <typename RndIt, typename URNG>
inline void shuffle(RndIt first, RndIt last, URNG&& reng)
{
  using std::swap;
  for (auto i = (last - first) - 1; i > 0; --i) {
    uniform_signed_distribution<decltype(i)> d(0, i);
    swap(first[i], first[d(reng)]);
  }
}


template <typename RndIt, typename SizeT, typename URNG>
inline void shuffle_n(RndIt first, RndIt last, SizeT n, URNG&& reng)
{
  using std::swap;
  using diff_t = decltype(last - first);
  const diff_t R = (last - first) - 1;
  const diff_t N = std::min(static_cast<diff_t>(n), R);
  for (diff_t i = 0; i < N; ++i) {
    uniform_signed_distribution<diff_t> d(i, R);
    swap(first[i], first[d(reng)]);
  }
}
}


//
// specialize std::generate_canonical<> for the generators provided in this library
// note that we can't specialize for types in std (undefined behavior),
// e.g. RNDUTILS_FAST_GENERATE_STD_CANONICAL(std::knuthb) would be illegal.
// Sad enough, because it would be much faster on some platforms.
//


#define RNDUTILS_FAST_GENERATE_CANONICAL_REAL_BITS(Real, Bits, URNG)     \
  template <>                                                            \
  inline Real generate_canonical<Real, Bits, URNG>(URNG & reng) noexcept \
  {                                                                      \
    return rndutils::detail::generate_canonical<Real, Bits, URNG>(reng); \
  }


#define RNDUTILS_FAST_GENERATE_CANONICAL(URNG)                         \
  RNDUTILS_FAST_GENERATE_CANONICAL_REAL_BITS(float, size_t(-1), URNG)  \
  RNDUTILS_FAST_GENERATE_CANONICAL_REAL_BITS(double, size_t(-1), URNG) \
  RNDUTILS_FAST_GENERATE_CANONICAL_REAL_BITS(long double, size_t(-1), URNG)


#if defined(RNDUTILS_ENABLE_FAST_STD_GENERATE_CANONICAL)
namespace std {
  RNDUTILS_FAST_GENERATE_CANONICAL(rndutils::xorshift128)
  RNDUTILS_FAST_GENERATE_CANONICAL(rndutils::xorshift1024)
}
#endif


#endif
