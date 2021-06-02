#ifndef MATH_UTILS_HPP_INCLUDED
#define MATH_UTILS_HPP_INCLUDED

#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glmutils/perp_dot.hpp"
#include "rndutils.hpp"



// let us see the assertion-messages in debug mode
#if defined(_DEBUG) || !defined(NDEBUG)
#define math_noexcept
#else
#define math_noexcept noexcept
#endif


namespace math {


  template <typename T>
  T constexpr pi = T(3.1415926535897932384626433832795);

  // statistics
  template <typename T>
  constexpr std::vector<T> normalize_vector(const std::vector<T>& a) 
  {
    auto minp = std::min(a);
    auto maxp = std::max(a);
    auto retv = a;

    std::for_each(retv.begin(), retv.end(), [](T& el) { normalize(el, minp, maxp); });
    std::for_each(retv.begin(), retv.end(), [](T& el) { assert(el <= 1); });

    return retv;
  }

  //template <typename T, typename X, size_t N>
  //constexpr void normalize_array(T (&a)[N])
  //{
  //  auto minp = std::min(a);
  //  auto maxp = std::max(a);

  //  std::for_each(a.size(), a.capacity(), [](X& el) { normalize(el, minp, maxp); });
  //  std::for_each(a.size(), a.capacity(), [](X& el) { assert(el <= 1); });
  //}


  template <typename T>
  constexpr T normalize(const T& a, const T& min, const T& max) math_noexcept
  {
    return (a - min) / (max - min);
  }


  // Geometry
  
  // returns angle [rad] between a and b clamped to [-rad(maxDeg), +rad(maxDeg)].
  template <typename T>
  constexpr T rad_between(const glm::tvec2<T>& a, const glm::tvec2<T>& b, T maxDeg = T(180)) math_noexcept
  {
    auto c = glmutils::perpDot(a, b);
    auto d = glm::dot(a, b);
    auto maxRad = glm::radians(maxDeg);
    return glm::clamp(std::atan2(c, d), -maxRad, +maxRad);
  }


  // returns angle [rad] between a and b clamped to [-rad(maxDeg), +rad(maxDeg)].
  template <typename T>
  constexpr T rad_between_max_rad(const glm::tvec2<T>& a, const glm::tvec2<T>& b, T maxRad = T(3.14159)) math_noexcept
  {
    auto c = glmutils::perpDot(a, b);
    auto d = glm::dot(a, b);
    return glm::clamp(std::atan2(c, d), -maxRad, +maxRad);
  }

  template <typename T>
  constexpr glm::tvec2<T> rotate(const glm::tvec2<T>& a, T rad) math_noexcept
  {
    const auto c = std::cos(rad);
    const auto s = std::sin(rad);
    return glm::tvec2<T>(a.x * c - a.y * s, a.x * s + a.y * c);
  }


  template <typename T>
  constexpr glm::tvec2<T> save_normalize(const glm::tvec2<T>& a, const glm::tvec2<T>& fallBack) math_noexcept
  {
    auto len2 = glm::dot(a, a);   // length2
    return (len2 > T(0.0000001)) ? a / std::sqrt(len2) : fallBack;  // covers NaN -> fallBack
  }

  template <typename T>
  constexpr inline T lerp(const T& a, const T& b, const T& mix) math_noexcept
  {
    return (T(1) - mix) * a + mix * b;
  }


  template <typename T>
  constexpr glm::tvec2<T> lerp(const glm::tvec2<T>& a, const glm::tvec2<T>& b, T mix) math_noexcept
  {
    return (glm::tvec2<T>(1) - mix) * a + mix * b;
  }


  template <typename T>
  constexpr glm::tvec2<T> slerp(const glm::tvec2<T>& a, const glm::tvec2<T>& b, T mix) math_noexcept
  {
    const auto theta = rad_between(a, b);
    return rotate(a, mix * theta);
  }

  template <typename T>
  constexpr glm::tvec2<T> slerp_rad_max(const glm::tvec2<T>& a, const glm::tvec2<T>& b, T mix, T maxRad = T(3.14159)) math_noexcept
  {
    const auto theta = rad_between_max_rad(a, b, maxRad);
    return rotate(a, mix * theta);
  }


  // Interpolation functions

  template <typename T>
  constexpr T fade(T x) math_noexcept
  {
    return x * x * x * (x * (x * 6 - 15) + 10);
  }

  template <typename T>
  constexpr T smootherstep(T x, const T edge0, const T edge1) math_noexcept
  {
    // Scale, bias and saturate x to 0..1 range
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
    return fade(x);
  }

  template <typename T>
  constexpr T smoothstep(T x, const T edge0, const T edge1) math_noexcept
  {
    // Scale, bias and saturate x to 0..1 range
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);

    return x * x * (3 - 2 * x);
  }

}

#endif
