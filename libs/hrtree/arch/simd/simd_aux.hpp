// hrtree/arch/simd/aux.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ARCH_SIMD_SIMD_AUX_HPP_INCLUDED
#define HRTREE_ARCH_SIMD_SIMD_AUX_HPP_INCLUDED

#include <hrtree/arch/select.hpp>

#if (defined(HRTREE_HAS_AVX) || defined(HRTREE_HAS_SSE2))


namespace hrtree { namespace simd {
      

  template <size_t Dim, typename V, size_t A> struct load_ {};
  template <size_t A> struct load_<2, __m128, A>   { static inline __m128 apply(const void* p) { return _mm_castpd_ps(_mm_load_sd((const double*)p)); } };
#ifdef HRTREE_HAS_AVX
  template <size_t A> struct load_<3, __m128, A>   { static inline __m128 apply(const void* p) { return _mm_maskload_ps((const float*)p, _mm_set_epi32(0,-1,-1,-1)); } };
#else
  template <size_t A> struct load_<3, __m128, A>   { static inline __m128 apply(const void* p) { return _mm_movelh_ps(_mm_castpd_ps(_mm_load_sd((const double*)p)), _mm_load_ss(((const float*)p) + 2)); } };
#endif
  template <size_t A> struct load_<4, __m128, A>   { static inline __m128 apply(const void* p) { return _mm_loadu_ps((const float*)p); } };
  template <>         struct load_<4, __m128, 16>  { static inline __m128 apply(const void* p) { return _mm_loadu_ps((const float*)p); } };
  template <>         struct load_<4, __m128, 32>  { static inline __m128 apply(const void* p) { return _mm_loadu_ps((const float*)p); } };
  template <size_t A> struct load_<2, __m128d, A>  { static inline __m128d apply(const void* p) { return _mm_loadu_pd((const double*)p); } };
  template <>         struct load_<2, __m128d, 16> { static inline __m128d apply(const void* p) { return _mm_load_pd((const double*)p); } };
  template <>         struct load_<2, __m128d, 32> { static inline __m128d apply(const void* p) { return _mm_load_pd((const double*)p); } };
#ifdef HRTREE_HAS_AVX
  template <size_t A> struct load_<3, __m256d, A>  { static inline __m256d apply(const void* p) { return _mm256_maskload_pd((const double*)p, _mm256_set_epi64x(0,-1,-1,-1)); } };
  template <size_t A> struct load_<4, __m256d, A>  { static inline __m256d apply(const void* p) { return _mm256_loadu_pd((const double*)p); } };
  template <>         struct load_<4, __m256d, 32> { static inline __m256d apply(const void* p) { return _mm256_load_pd((const double*)p); } };
#endif

  template <typename Point>
  inline typename simd_type<Point>::type load(const Point& p)
  {
    typedef ::hrtree::traits::point_access<Point> pa;
    return load_< ::hrtree::traits::point_dim<Point>::value, typename simd_type<Point>::type, HRTREE_ALIGNOF(Point) >::apply(pa::ptr(p));
  }


  template <size_t Dim, typename V, size_t A> struct store_ {};
  template <size_t A> struct store_<2, __m128, A>   { static inline void apply(void* p, __m128 x) { _mm_store_sd((double*)p, _mm_castps_pd(x)); } };
#ifdef HRTREE_HAS_AVX
  template <size_t A> struct store_<3, __m128, A>   { static inline void apply(void* p, __m128 x) { _mm_maskstore_ps((float*)p, _mm_set_epi32(0,-1,-1,-1), x); } };
#else
  template <size_t A> struct store_<3, __m128, A>   { static inline void apply(void* p, __m128 x) { _mm_store_sd((double*)p, _mm_castps_pd(x)); _mm_store_ss(((float*)p) + 2, _mm_movehl_ps(x, x)); } };
//  template <size_t A> struct store_<3, __m128, A>   { static inline void apply(void* p, __m128 x) { _mm_maskmoveu_si128(_mm_castps_si128(x), _mm_set_epi32(-1,-1,-1,0), (char*)p); } };
#endif
  template <size_t A> struct store_<4, __m128, A>   { static inline void apply(void* p, __m128 x) { _mm_storeu_ps((float*)p, x); } };
  template <>         struct store_<4, __m128, 16>  { static inline void apply(void* p, __m128 x) { _mm_store_ps((float*)p, x); } };
  template <>         struct store_<4, __m128, 32>  { static inline void apply(void* p, __m128 x) { _mm_store_ps((float*)p, x); } };
  template <size_t A> struct store_<2, __m128d, A>  { static inline void apply(void* p, __m128d x) { _mm_storeu_pd((double*)p, x); } };
  template <>         struct store_<2, __m128d, 16> { static inline void apply(void* p, __m128d x) { _mm_store_pd((double*)p, x); } };
  template <>         struct store_<2, __m128d, 32> { static inline void apply(void* p, __m128d x) { _mm_store_pd((double*)p, x); } };
#ifdef HRTREE_HAS_AVX
  template <size_t A> struct store_<3, __m256d, A>  { static inline void apply(void* p, __m256d x) { _mm256_maskstore_pd((double*)p, _mm256_set_epi64x(0,-1,-1,-1), x); } };
  template <size_t A> struct store_<4, __m256d, A>  { static inline void apply(void* p, __m256d x) { _mm256_storeu_pd((double*)p, x); } };
  template <>         struct store_<4, __m256d, 32> { static inline void apply(void* p, __m256d x) { _mm256_store_pd((double*)p, x); } };
#endif

  template <typename Point>
  inline void store(Point& p, typename simd_type<Point>::type x)
  {
    typedef ::hrtree::traits::point_access<Point> pa;
    return store_< ::hrtree::traits::point_dim<Point>::value, typename simd_type<Point>::type, HRTREE_ALIGNOF(Point) >::apply(pa::ptr(p), x);
  }


  template <typename V>
  struct set;

  template <> struct set<__m128>
  {
    static inline __m128 zero() { return _mm_setzero_ps(); }
    template <typename S>  static inline __m128 val(S value) { return _mm_set1_ps(float(value)); }
  };

  template <> struct set<__m128d>
  {
    static inline __m128d zero() { return _mm_setzero_pd(); }
    template <typename S>  static inline __m128d val(S value) { return _mm_set1_pd(double(value)); }
  };

#ifdef HRTREE_HAS_AVX
  template <> struct set<__m256d>
  {
    static inline __m256d zero() { return _mm256_setzero_pd(); }
    template <typename S>  static inline __m256d val(S value) { return _mm256_set1_pd(double(value)); }
  };
#endif


  inline __m128 mul(__m128 a, __m128 b)   { return _mm_mul_ps(a, b); }
  inline __m128 add(__m128 a, __m128 b)   { return _mm_add_ps(a, b); }
  inline __m128 sub(__m128 a, __m128 b)   { return _mm_sub_ps(a, b); }
  inline __m128 div(__m128 a, __m128 b)   { return _mm_div_ps(a, b); }
  inline __m128 min(__m128 a, __m128 b)   { return _mm_min_ps(a, b); }
  inline __m128 max(__m128 a, __m128 b)   { return _mm_max_ps(a, b); }
  inline __m128 band(__m128 a, __m128 b)   { return _mm_and_ps(a, b); }
  inline __m128 cmpge(__m128 a, __m128 b) { return _mm_cmpge_ps(a, b); }
  inline __m128 cmple(__m128 a, __m128 b) { return _mm_cmple_ps(a, b); }
  inline __m128i cvtt_epi32(__m128 a)     { return _mm_cvttps_epi32(a); }
  inline int movemask(__m128 a)           { return _mm_movemask_ps(a); }

  inline __m128d mul(__m128d a, __m128d b)   { return _mm_mul_pd(a, b); }
  inline __m128d add(__m128d a, __m128d b)   { return _mm_add_pd(a, b); }
  inline __m128d sub(__m128d a, __m128d b)   { return _mm_sub_pd(a, b); }
  inline __m128d div(__m128d a, __m128d b)   { return _mm_div_pd(a, b); }
  inline __m128d min(__m128d a, __m128d b)   { return _mm_min_pd(a, b); }
  inline __m128d max(__m128d a, __m128d b)   { return _mm_max_pd(a, b); }
  inline __m128d band(__m128d a, __m128d b)   { return _mm_and_pd(a, b); }
  inline __m128d cmpge(__m128d a, __m128d b) { return _mm_cmpge_pd(a, b); }
  inline __m128d cmple(__m128d a, __m128d b) { return _mm_cmple_pd(a, b); }
  inline __m128i cvtt_epi32(__m128d a)       { return _mm_cvttpd_epi32(a); }
  inline int movemask(__m128d a)             { return _mm_movemask_pd(a); }


#ifdef HRTREE_HAS_AVX
  inline __m256d add(__m256d a, __m256d b)   { return _mm256_add_pd(a, b); }
  inline __m256d sub(__m256d a, __m256d b)   { return _mm256_sub_pd(a, b); }
  inline __m256d mul(__m256d a, __m256d b)   { return _mm256_mul_pd(a, b); }
  inline __m256d div(__m256d a, __m256d b)   { return _mm256_div_pd(a, b); }
  inline __m256d min(__m256d a, __m256d b)   { return _mm256_min_pd(a, b); }
  inline __m256d max(__m256d a, __m256d b)   { return _mm256_max_pd(a, b); }
  inline __m256d band(__m256d a, __m256d b)   { return _mm256_and_pd(a, b); }
  inline __m256d cmpge(__m256d a, __m256d b) { return _mm256_cmp_pd(a, b, _CMP_GE_OQ); }
  inline __m256d cmple(__m256d a, __m256d b) { return _mm256_cmp_pd(a, b, _CMP_LE_OQ); }
  inline __m128i cvtt_epi32(__m256d a)       { return _mm256_cvttpd_epi32(a); }
  inline int movemask(__m256d a)             { return _mm256_movemask_pd(a); }
#endif


}}


#endif
#endif



