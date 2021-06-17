// hrtree/arch/select.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ARCH_SELECT_HPP_INCLUDED
#define HRTREE_ARCH_SELECT_HPP_INCLUDED

#include <hrtree/config.hpp>
#include <hrtree/adapt_point.hpp>
#ifdef HRTREE_HAS_AVX
  #include <immintrin.h>
#elif defined(HRTREE_HAS_SSE2)
  #include <xmmintrin.h>
  #include <emmintrin.h>
#endif



namespace hrtree {
      
  struct unsupported {};
  template <size_t Dim, typename Scalar> struct simd_type_impl { typedef unsupported type; };

#if (defined (HRTREE_HAS_AVX) || defined(HRTREE_HAS_SSE2))
  template <>  struct simd_type_impl<2,float> { typedef __m128 type; };
  template <>  struct simd_type_impl<3,float> { typedef __m128 type; };
  template <>  struct simd_type_impl<4,float> { typedef __m128 type; };
  template <>  struct simd_type_impl<2,double> { typedef __m128d type; };
#ifdef HRTREE_HAS_AVX
  template <>  struct simd_type_impl<3,double> { typedef __m256d type; };
  template <>  struct simd_type_impl<4,double> { typedef __m256d type; };
#endif
#endif

  template <typename Point>
  struct simd_type
  {
    typedef typename simd_type_impl< traits::point_dim<Point>::value, typename traits::point_scalar<Point>::type >::type type;
  };

}


#endif



