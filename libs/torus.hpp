#ifndef TOM_TORUS_HPP_INCLUDED
#define TOM_TORUS_HPP_INCLUDED

#include <cassert>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glmutils/perp_dot.hpp"
#include "rndutils.hpp"
#include "math.hpp"

// Hanno 2018


// let us see the assertion-messages in debug mode
#if defined(_DEBUG) || !defined(NDEBUG)
#define torus_noexcept
#else
#define torus_noexcept noexcept
#endif


namespace torus {


  template <typename T>
  constexpr bool is_wrapped(T WH, T x) torus_noexcept
  {
    return (x >= T(0)) && (x <= WH);
  }


  template <typename T>
  constexpr bool is_wrapped(T WH, const glm::tvec2<T>& pos) torus_noexcept
  {
    return is_wrapped(WH, pos.x) && is_wrapped(WH, pos.y);
  }


  // torus wrap, coordinate [0,WH]
  template <typename T>
  constexpr T wrap(T WH, T x) torus_noexcept
  { 
    // we need 'positive modulus' here:
    const auto res = glm::mod(x, WH);
    assert(is_wrapped(WH, res));
    return res;
  }


  // torus wrap, vector ([0,WH], [0,WH])
  template <typename T>
  constexpr glm::tvec2<T> wrap(T WH, const glm::tvec2<T>& pos) torus_noexcept
  {
    return glm::tvec2<T>(wrap(WH, pos.x), wrap(WH, pos.y));
  }


  // torus distance
  template <typename T>
  constexpr T distance2(T WH, const glm::tvec2<T>& a, const glm::tvec2<T>& b) torus_noexcept
  {
    assert(is_wrapped(WH, a));
    assert(is_wrapped(WH, b));
    const auto ad = glm::abs(a - b);
    return glm::length2( glm::min(ad, WH - ad) );
  }


  // torus distance
  template <typename T>
  constexpr T distance(T WH, const glm::tvec2<T>& a, const glm::tvec2<T>& b) torus_noexcept
  {
    return std::sqrt(distance2(WH, a, b));
  }


  template <typename T>
  constexpr T ofs_coor(T WH, T a, T b) torus_noexcept
  {
    assert(is_wrapped(WH, a));
    assert(is_wrapped(WH, b));
    using std::abs;
    const auto a0 = b - a;
    const auto a1 = a0 + WH;  // (b + WH) - a
    const auto a2 = a0 - WH;  // (b - WH) - a
    return (abs(a0) < abs(a1))
      ? ((abs(a0) < abs(a2)) ? a0 : a2)
      : ((abs(a1) < abs(a2)) ? a1 : a2);
  }


  template <typename T>
  constexpr glm::tvec2<T> ofs(T WH, const glm::tvec2<T>& a, const glm::tvec2<T>& b) torus_noexcept
  {
    return { ofs_coor(WH, a.x, b.x), ofs_coor(WH, a.y, b.y) };
  }
  
  template <typename T>
  constexpr glm::tvec2<T> lerp(T WH, const glm::tvec2<T>& a, const glm::tvec2<T>& b, T mix) torus_noexcept
  {
    const auto c = a + ofs(WH, a, b);
    return wrap(WH, (T(1) - mix) * a + mix * c);
  }


  template <typename T> // if individual b is behind from individual a
  constexpr bool is_behind(const glm::tvec2<T>& a_p, const glm::tvec2<T>& a_h, const glm::tvec2<T>& b_p, T WH) torus_noexcept
  {
    return glm::dot(a_h, ofs(WH, a_p, b_p)) < 0.;
  }


  template <typename T, typename A> // if individual b is behind from individual a
  constexpr bool is_atside(const glm::tvec2<T>& a_p, const glm::tvec2<T>& a_h, const glm::tvec2<T>& b_p, const A side_angle, T WH) torus_noexcept
  {
    const auto cFrontFov = std::cos(glm::radians(180.0f - 0.5f * (360.0f - side_angle))); // in-front field of view
    const auto ddist = std::sqrt(torus::distance2(WH, a_p, b_p));

    return glm::dot(a_h, ofs(WH, a_p, b_p)) > ddist * cFrontFov ? false : true; // if not infront it will be at the side
  }

}

#endif
