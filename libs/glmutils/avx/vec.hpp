#ifndef GLMUTILS_AVX_VEC_HPP_INCLUDED
#define GLMUTILS_AVX_VEC_HPP_INCLUDED


#include <immintrin.h>
#include <type_traits>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "float.hpp"


namespace glmutils { namespace avx {


  template <typename GLM_VEC> class vec_t;


  namespace detail {

    template <typename T> struct cmp_mask {};
    template <> struct cmp_mask< glm::vec2 > { static const int value = 0x3f; };
    template <> struct cmp_mask< glm::vec3 > { static const int value = 0x7f; };
    template <> struct cmp_mask< glm::vec4 > { static const int value = 0xff; };


    template <typename T> struct dp_mask {};
    template <> struct dp_mask< glm::vec2 > { static const int value = 0x3f; };
    template <> struct dp_mask< glm::vec3 > { static const int value = 0x7f; };
    template <> struct dp_mask< glm::vec4 > { static const int value = 0xff; };


    template <typename T> struct permute_ctrl_f {};
    template <> struct permute_ctrl_f< glm::vec2 > { static const int value = 0xf0; };
    template <> struct permute_ctrl_f< glm::vec3 > { static const int value = 0xc0; };
    template <> struct permute_ctrl_f< glm::vec4 > { static const int value = 0x00; };


    template <typename T> struct blend_mask {};
    template <> struct blend_mask< glm::vec2 > { static const int value = 0x3; };
    template <> struct blend_mask< glm::vec3 > { static const int value = 0x7; };
    template <> struct blend_mask< glm::vec4 > { static const int value = 0xf; };


    template <typename T> struct epi32_mask {};
    template <> struct epi32_mask< glm::vec2 > { static __m128i value() { return _mm_set_epi32(0,0,-1,-1); } };
    template <> struct epi32_mask< glm::vec3 > { static __m128i value() { return _mm_set_epi32(0,-1,-1,-1); } };
    template <> struct epi32_mask< glm::vec4 > { static __m128i value() { return _mm_set_epi32(-1,-1,-1,-1); } };


    template <typename T, int N> struct has_component { static const bool value = false; };
    template <> struct has_component< glm::vec2, 1 > {  static const bool value = true; };
    template <> struct has_component< glm::vec2, 2 > {  static const bool value = true; };
    template <> struct has_component< glm::vec3, 1 > {  static const bool value = true; };
    template <> struct has_component< glm::vec3, 2 > {  static const bool value = true; };
    template <> struct has_component< glm::vec3, 3 > {  static const bool value = true; };
    template <> struct has_component< glm::vec4, 1 > {  static const bool value = true; };
    template <> struct has_component< glm::vec4, 2 > {  static const bool value = true; };
    template <> struct has_component< glm::vec4, 3 > {  static const bool value = true; };
    template <> struct has_component< glm::vec4, 4 > {  static const bool value = true; };


    template <typename GLM_VEC>
    inline __m128 set(float x)
    {
      return _mm_permute_ps(_mm_set_ss(x), permute_ctrl_f<GLM_VEC>::value);
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 2>::value, __m128
    >::type set(float x, float y)
    {
      return _mm_set_ps(0, 0, y, x);
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 3>::value, __m128
    >::type set(float x, float y, float z)
    {
      return _mm_set_ps(0, z, y, x);
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 4>::value, __m128
    >::type set(float x, float y, float z, float w)
    {
      return _mm_set_ps(w, z, y, x);
    }


    template <typename GLM_VEC>
    inline __m128 set(__m128 src) 
    { 
      return _mm_blend_ps(_mm_setzero_ps(), src, blend_mask<GLM_VEC>::value); 
    }


    template <>
    inline __m128 set<glm::vec4>(__m128 src) 
    { 
      return src; 
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 4>::value, __m128
    >::type set_w(__m128 v, __m128 x)
    {
      __m128 a = _mm_shuffle_ps( v, x, _MM_SHUFFLE(0,1,2,3) );
      return _mm_shuffle_ps( v, a, _MM_SHUFFLE(3,1,1,0));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 3>::value, __m128
    >::type set_z(__m128 v, __m128 x)
    {
      __m128 a = _mm_shuffle_ps( v, x, _MM_SHUFFLE(0,1,2,3) );
      return _mm_shuffle_ps( v, a, _MM_SHUFFLE(0,3,1,0));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 2>::value, __m128
    >::type set_y(__m128 v, __m128 x)
    {
      __m128 a = _mm_movelh_ps(v, x);
      return _mm_shuffle_ps( a, v, _MM_SHUFFLE(3,2,2,0));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 1>::value, __m128
    >::type set_x(__m128 v, __m128 x)
    {
      __m128 a = _mm_movelh_ps(v, x);
      return _mm_shuffle_ps( a, v, _MM_SHUFFLE(3,2,1,2));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 4>::value, float
    >::type get_w(__m128 v)
    {
      __m128 x = _mm_movehl_ps(_mm_setzero_ps(), v);
      return _mm_cvtss_f32(_mm_shuffle_ps(x, x, _MM_SHUFFLE(2,2,2,1)));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 3>::value, float
    >::type get_z(__m128 v)
    {
      return _mm_cvtss_f32(_mm_movehl_ps(v, v));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 2>::value, float
    >::type get_y(__m128 v)
    {
      __m128 x = _mm_movelh_ps(v, v);
      return _mm_cvtss_f32(_mm_shuffle_ps(x, x, _MM_SHUFFLE(0,0,0,3)));
    }


    template <typename GLM_VEC>
    inline typename std::enable_if< 
      has_component<GLM_VEC, 1>::value, float
    >::type get_x(__m128 v)
    {
      return _mm_cvtss_f32(v);
    }


    template <typename GLM_VEC>
    inline __m128 load(float* src) 
    { 
      return _mm_maskload_ps(src, epi32_mask<GLM_VEC>::value()); 
    }


    template <typename GLM_VEC>
    inline __m128 load(GLM_VEC const& src) 
    { 
      return load<GLM_VEC>((float*)glm::value_ptr(src)); 
    }


    template <typename GLM_VEC>
    inline void store(float* dst, __m128 x) 
    { 
      _mm_maskstore_ps(dst, epi32_mask<GLM_VEC>::value(), x); 
    }


    template <typename GLM_VEC>
    inline void store(GLM_VEC const& dst, __m128 x) 
    { 
      store<GLM_VEC>((float*)glm::value_ptr(dst), x); 
    }


    template <typename GLM_VEC>
    inline GLM_VEC cast(__m128 x) 
    { 
      __declspec (align(16)) GLM_VEC res;
      store<GLM_VEC>(res, x);
      return res;
    }


    template <typename GLM_VEC>
    inline __m128 dot(__m128 a, __m128 b) 
    { 
      return _mm_dp_ps(a, b, dp_mask<GLM_VEC>::value); 
    }


    template <typename GLM_VEC>
    inline __m128 fast_normalize(__m128 x) 
    { 
      return _mm_mul_ps(x, _mm_rsqrt_ps(dot<GLM_VEC>(x, x))); 
    }


    // reciprocal sqrt plus one Newton-Rhapson step
    template <typename GLM_VEC>
    inline __m128 normalize(__m128 x) 
    { 
      __m128 dp = dot<GLM_VEC>(x, x);
      __m128 y0 = _mm_rsqrt_ps(dp); 
      // Newton-Rhapson step: reciprocal length y1 = 0.5 * y0 * (3 - y0 * y0 * dp)
      __m128 muls = _mm_mul_ps(_mm_mul_ps(y0, y0), dp); 
      __m128 y1 = _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), y0), _mm_sub_ps(_mm_set1_ps(3.0f), muls));
      return _mm_mul_ps(x, y1);
    }


    template <typename GLM_VEC>
    inline __m128 save_normalize(__m128 x, __m128 fallback) 
    { 
      __m128 dp = dot<GLM_VEC>(x, x);
      __m128 y0 = _mm_rsqrt_ps(dp); 
      // Test for +- Inf
      if (0xf == _mm_movemask_ps(_mm_cmp_ss(y0, y0, _CMP_ORD_Q)))
      {
        return fallback;
      }
      // Newton-Rhapson step: reciprocal length y1 = 0.5 * y0 * (3 - y0 * y0 * dp)
      __m128 t = _mm_mul_ps(_mm_mul_ps(y0, y0), dp); 
      __m128 y1 = _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), y0), _mm_sub_ps(_mm_set1_ps(3.0f), t));
      return _mm_mul_ps(x, y1);
    }

    template <typename GLM_VEC>
    inline __m128 fast_normalize_length(__m128 x, float& len) 
    { 
      __m128 dp = dot<GLM_VEC>(x, x);
      __m128 y0 = _mm_rsqrt_ps(dp);
      len = _mm_cvtss_f32(_mm_mul_ss(dp, y0));
      return _mm_mul_ps(x, y0); 
    }


    // reciprocal sqrt plus one Newton-Rhapson step
    template <typename GLM_VEC>
    inline __m128 normalize_length(__m128 x, float& len) 
    { 
      __m128 dp = dot<GLM_VEC>(x, x);
      __m128 y0 = _mm_rsqrt_ps(dp); 
      // Newton-Rhapson step: reciprocal length y1 = 0.5 * y0 * (3 - y0 * y0 * dp)
      __m128 muls = _mm_mul_ps(_mm_mul_ps(y0, y0), dp); 
      __m128 y1 = _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), y0), _mm_sub_ps(_mm_set1_ps(3.0f), muls));
      len = _mm_cvtss_f32(_mm_mul_ss(dp, y1));
      return _mm_mul_ps(x, y1);
    }


    template <typename GLM_VEC>
    inline __m128 save_normalize_length(__m128 x, __m128 fallback, float& len) 
    { 
      __m128 dp = dot<GLM_VEC>(x, x);
      __m128 y0 = _mm_rsqrt_ps(dp); 
      // Test for +- Inf
      if (0xf == _mm_movemask_ps(_mm_cmp_ss(y0, y0, _CMP_ORD_Q)))
      {
        len = 0.0f;
        return fallback;
      }
      // Newton-Rhapson step: reciprocal length y1 = 0.5 * y0 * (3 - y0 * y0 * dp)
      __m128 t = _mm_mul_ps(_mm_mul_ps(y0, y0), dp); 
      __m128 y1 = _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), y0), _mm_sub_ps(_mm_set1_ps(3.0f), t));
      len = _mm_cvtss_f32(_mm_mul_ss(dp, y1));
      return _mm_mul_ps(x, y1);
    }

  }


  template <typename GLM_VEC>
  class vec_t
  {
  public:
    typedef GLM_VEC glm_type;

    vec_t() {}
    vec_t(const vec_t& x) { m128 = x.m128; }
    explicit vec_t(__m128 x) { m128 = detail::set<glm_type>(x); }
    vec_t(__m128 x, bool) { m128 = x; }
    vec_t(glm_type const& x) { m128 = detail::load<glm_type>(x); }
    explicit vec_t(float x) { m128 = detail::set<glm_type>(x); }
    vec_t(float x, float y) { m128 = detail::set<glm_type>(x, y); }
    vec_t(float x, float y, float z) { m128 = detail::set<glm_type>(x,y,z); }
    vec_t(float x, float y, float z, float w) { m128 = detail::set<glm_type>(x,y,z,w); }
    vec_t(glm::vec3 const & v3, float w) { m128 = detail::set_w<glm_type>(detail::load(v3), _mm_set_ss(w)); }
    vec_t(vec_t<glm::vec3> const & v3, float w) { m128 = detail::set_w<glm_type>(v3.m128, _mm_set_ss(w)); }


    void store(glm_type& dst) { detail::store<glm_type>(dst, m128); }
    void store(float* dst) { detail::store<glm_type>(dst, m128); }

    vec_t& operator = (float x) { m128 = detail::set<glm_type>(x); return *this; }
    vec_t& operator = (const glm::vec3& x) { m128 = detail::load<glm_type>(x); return *this; }
    vec_t& operator = (const vec_t& x) { m128 = x.m128; return *this; }
    vec_t& operator += (const vec_t& x) { m128 = _mm_add_ps(m128, x.m128); return *this; }
    vec_t& operator += (float x) { m128 = _mm_add_ps(m128, _mm_set_ps(0,x,x,x)); return *this; }
    vec_t& operator -= (const vec_t& x) { m128 = _mm_sub_ps(m128, x.m128); return *this; }
    vec_t& operator -= (float x) { m128 = _mm_sub_ps(m128, _mm_set_ps(0,x,x,x)); return *this; }
    vec_t& operator *= (const vec_t& x) { m128 = _mm_mul_ps(m128, x.m128); return *this; }
    vec_t& operator *= (float x) { m128 = _mm_mul_ps(m128, _mm_set_ps1(x)); return *this; }
    vec_t& operator /= (const vec_t& x) { m128 = _mm_div_ps(m128, x.m128); return *this; }
    vec_t& operator /= (float x) { m128 = _mm_div_ps(m128, _mm_set_ps1(x)); return *this; }


    float get_x() const { return detail::get_x<glm_type>(m128); }
    float get_y() const { return detail::get_y<glm_type>(m128); }
    float get_z() const { return detail::get_z<glm_type>(m128); }
    float get_w() const { return detail::get_w<glm_type>(m128); }


    vec_t& set_x(float x) { m128 = detail::set_x<glm_type>(m128, _mm_set_ss(x)); return *this; }
    vec_t& set_y(float x) { m128 = detail::set_y<glm_type>(m128, _mm_set_ss(x)); return *this; }
    vec_t& set_z(float x) { m128 = detail::set_z<glm_type>(m128, _mm_set_ss(x)); return *this; }
    vec_t& set_w(float x) { m128 = detail::set_w<glm_type>(m128, _mm_set_ss(x)); return *this; }


    __m128 m128;
  };


  template <typename GLM_VEC, typename AVX_VEC>
  inline GLM_VEC cast(AVX_VEC const& x)
  {
    return detail::cast<GLM_VEC>(x.m128);
  }


  template <typename GLM_VEC>
  inline GLM_VEC cast(vec_t<GLM_VEC> const& x)
  {
    return detail::cast<GLM_VEC>(x.m128);
  }


  template <typename T> inline bool operator == (vec_t<T> const& a, vec_t<T> const& b) 
  { 
    return 0xf == _mm_movemask_ps(_mm_cmpeq_ps(a.m128, b.m128)); 
  }
  template <typename T> inline bool operator != (vec_t<T> const& a, vec_t<T> const& b) { return ! (a == b); }

  template <typename T> inline vec_t<T> operator - (vec_t<T> const& x) { return vec_t<T>(_mm_sub_ps(_mm_setzero_ps(), x.m128), false); }
  template <typename T> inline vec_t<T> operator + (vec_t<T> const& a, vec_t<T> const& b) { return vec_t<T>(a) += b; }
  template <typename T> inline vec_t<T> operator + (vec_t<T> const& a, float b) { return vec_t<T>(a) += b; }
  template <typename T> inline vec_t<T> operator + (float a, vec_t<T> const& b) { return vec_t<T>(a) += b; }
  template <typename T> inline vec_t<T> operator - (vec_t<T> const& a, vec_t<T> const& b) { return vec_t<T>(a) -= b; }
  template <typename T> inline vec_t<T> operator - (vec_t<T> const& a, float b) { return vec_t<T>(a) -= b; }
  template <typename T> inline vec_t<T> operator - (float a, vec_t<T> const& b) { return vec_t<T>(a) -= b; }
  template <typename T> inline vec_t<T> operator * (vec_t<T> const& a, vec_t<T> const& b) { return vec_t<T>(a) *= b; }
  template <typename T> inline vec_t<T> operator * (vec_t<T> const& a, float b) { return vec_t<T>(a) *= b; }
  template <typename T> inline vec_t<T> operator * (float a, vec_t<T> const& b) { return vec_t<T>(b) *= a; }
  template <typename T> inline vec_t<T> operator / (vec_t<T> const& a, vec_t<T> const& b) { return vec_t<T>(a) /= b; }
  template <typename T> inline vec_t<T> operator / (vec_t<T> const& a, float b) { return vec_t<T>(a) /= b; }
  template <typename T> inline vec_t<T> operator / (float a, vec_t<T> const& b) { return vec_t<T>(b) /= a; }

  template <typename T> inline float dot(vec_t<T> const& a, vec_t<T> const & b) { return _mm_cvtss_f32(detail::dot<T>(a.m128, b.m128)); }
  template <typename T> inline float length2(vec_t<T> const & x) { return dot(x, x); }
  template <typename T> inline float length(vec_t<T> const & x) { return sqrt(dot(x, x)); }
  template <typename T> inline float fast_length(vec_t<T> const & x) { return fast_sqrt(dot(x, x)); }
  template <typename T> inline vec_t<T> fast_normalize(vec_t<T> const& x) { return vec_t<T>(detail::fast_normalize<T>(x.m128), false); }
  template <typename T> inline vec_t<T> save_normalize(vec_t<T> const& x, vec_t<T> const& fallback) { return vec_t<T>(detail::save_normalize<T>(x.m128, fallback.m128), false); }
  template <typename T> inline vec_t<T> normalize(vec_t<T> const& x) { return vec_t<T>(detail::normalize<T>(x.m128), false); }
  template <typename T> inline vec_t<T> fast_normalize_length(vec_t<T> const& x, float& len) { return vec_t<T>(detail::fast_normalize_length<T>(x.m128, len), false); }
  template <typename T> inline vec_t<T> save_normalize_length(vec_t<T> const& x, vec_t<T> const& fallback, float& len) { return vec_t<T>(detail::save_normalize_length<T>(x.m128, fallback.m128, len), false); }
  template <typename T> inline vec_t<T> normalize_length(vec_t<T> const& x, float& len) { return vec_t<T>(detail::normalize_length<T>(x.m128, len), false); }


  template <typename T> inline vec_t<T> abs(vec_t<T> const& x)
  {
    const auto mask = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
    return vec_t<T>(_mm_and_ps(x.m128, mask));
  }
    
    
  inline vec_t<glm::vec3> cross(vec_t<glm::vec3> const& a, vec_t<glm::vec3> const& b)
  {  
    __m128 ma = a.m128;
    __m128 mb = b.m128;
    return vec_t<glm::vec3>( _mm_sub_ps(
      _mm_mul_ps(_mm_shuffle_ps(ma, ma, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(mb, mb, _MM_SHUFFLE(3, 1, 0, 2))), 
      _mm_mul_ps(_mm_shuffle_ps(ma, ma, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(mb, mb, _MM_SHUFFLE(3, 0, 2, 1)))
      ));
  }


  inline vec_t<glm::vec2> perpDot(vec_t<glm::vec2> const& x)
  {  
    __m128 t = _mm_shuffle_ps(x.m128, x.m128, _MM_SHUFFLE(1,0,0,1));
    __m128 p = _mm_mul_ps(t, _mm_set_ps(0,0,1,-1)); 
    return vec_t<glm::vec2>(p, false);
  }


  inline float perpDot(vec_t<glm::vec2> const& a, vec_t<glm::vec2> const& b)
  {  
    __m128 t = _mm_shuffle_ps(a.m128, a.m128, _MM_SHUFFLE(1,0,0,1));
    __m128 p = _mm_mul_ps(t, _mm_set_ps(0,0,1,-1)); 
    return _mm_cvtss_f32(detail::dot<glm::vec2>(p, b.m128));
  }


  template <typename T> inline vec_t<T> min(vec_t<T> const& a, vec_t<T> const& b)
  {
    return vec_t<T>( _mm_min_ps(a.m128, b.m128), false );
  }


  template <typename T> inline vec_t<T> max(vec_t<T> const& a, vec_t<T> const& b)
  {
    return vec_t<T>( _mm_max_ps(a.m128, b.m128), false );
  }


  template <typename T> inline vec_t<T> mix(vec_t<T> const& x, vec_t<T> const& y, float a)
  {
    return (1.0f - a) * x + a * y;
  }

  template <typename T> inline vec_t<T> clamp(vec_t<T> const& x, vec_t<T> const& minVal, vec_t<T> const& maxVal)
  {
     return avx::max(minVal, avx::min(x, maxVal));
  }
  template <typename T> inline vec_t<T> clamp(vec_t<T> const& x, float minVal, float maxVal)
  {
    return avx::max(avx::vec_t<T>(minVal), avx::min(x, vec_t<T>(maxVal)));
  }
  inline float clamp(float x, float minVal, float maxVal)
  {
    return _mm_cvtss_f32( _mm_max_ss(_mm_set_ss(minVal), _mm_min_ss(_mm_set_ss(x), _mm_set_ss(maxVal))) );
  }


  template <typename T> inline vec_t<T> saturate(vec_t<T> const& x)
  {
    return avx::clamp<T>(x, 0, 1);    
  }
  inline float saturate(float x)
  {
    return avx::clamp(x, 0, 1);
  }


  template <typename T> inline vec_t<T> smoothstep(vec_t<T> const& e0, vec_t<T> const& e1, vec_t<T> const& x)
  {
    vec_t<T> t = avx::saturate<T>((x-e0)/(e1-e0));
    return t*t*(vec_t<T>(3) - 2*t);
  }
  template <typename T> inline vec_t<T> smoothstep(float e0, float e1, vec_t<T> const& x)
  {
    return avx::smoothstep(vec_t<T>(e0), vec_t<T>(e1), x);
  }
  inline float smoothstep(float e0, float e1, float x)
  {
    float t = avx::saturate((x-e0)/(e1-e0));
    return t*t*(3 - 2*t);
  }


  template <typename T> inline vec_t<T> smootherstep(vec_t<T> const& e0, vec_t<T> const& e1, vec_t<T> const& x)
  {
    vec_t<T> t = avx::saturate<T>((x-e0)/(e1-e0));
    return t*t*t*(t*(t*vec_t<T>(6) - 15) + 10);
  }
  template <typename T> inline vec_t<T> smootherstep(float e0, float e1, vec_t<T> const& x)
  {
    return avx::smootherstep(vec_t<T>(e0), vec_t<T>(e1), x);
  }
  inline float smootherstep(float e0, float e1, float x)
  {
    float t = avx::saturate((x-e0)/(e1-e0));
    return  t*t*t*(t*(t*6 - 15) + 10);
  }


  typedef avx::vec_t<glm::vec2> vec2;
  typedef avx::vec_t<glm::vec3> vec3;
  typedef avx::vec_t<glm::vec4> vec4;
}}


#endif

