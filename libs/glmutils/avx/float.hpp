#ifndef GLMUTILS_AVX_FLOAT_HPP_INCLUDED
#define GLMUTILS_AVX_FLOAT_HPP_INCLUDED

#include <cmath>
#include <immintrin.h>


namespace glmutils { namespace avx {


  //! reciprocal sqrt
  inline float rsqrt(float x)
  {
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
  }


  //! x * reciprocal sqrt
  inline float fast_sqrt(float x)
  {
    return x * _mm_cvtss_f32( _mm_rsqrt_ss(_mm_set_ss(x)) );
  }


  inline float sqrt(float x)
  {
    return std::sqrt(x);
  }

}}

#endif
