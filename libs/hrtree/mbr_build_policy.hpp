// hrtree/mbr_build_policy.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_MBR_BUILD_POLICY_HPP_INCLUDED
#define HRTREE_MBR_BUILD_POLICY_HPP_INCLUDED

#include <hrtree/adapt_mbr.hpp>
#include <hrtree/arch/select.hpp>


namespace hrtree {

  namespace detail {

    template <typename simd_vec, typename Mbr>
    struct mbr_build_policy_impl;


    template <typename Mbr>
    struct mbr_build_policy_impl<unsupported, Mbr>
    {
      typedef typename ::hrtree::traits::point_type<Mbr>::type point;
      typedef typename ::hrtree::traits::point_scalar<point>::type scalar;

      template <typename IT, typename OIT>
      static void apply(IT first, IT last, OIT out)
      {
        scalar* lhs_min = ::hrtree::traits::get_ptr<0>(*out);
        scalar* lhs_max = ::hrtree::traits::get_ptr<1>(*out);
        for (; first != last; ++first)
        {
          const scalar* rhs_min = ::hrtree::traits::get_ptr<0>(*first);
          const scalar* rhs_max = ::hrtree::traits::get_ptr<1>(*first);
          for (std::size_t i=0; i < ::hrtree::traits::point_dim<point>::value; ++i)
          {
            if (lhs_min[i] > rhs_min[i]) lhs_min[i] = rhs_min[i];
            if (lhs_max[i] < rhs_max[i]) lhs_max[i] = rhs_max[i];
          }
        }
      }
    };

  }
  
  template <typename Mbr>
  class mbr_build_policy
  {
    typedef typename traits::point_type<Mbr>::type point;
    typedef detail::mbr_build_policy_impl< typename simd_type<point>::type, Mbr> impl_;

  public:
    mbr_build_policy() {}

    template <typename IT, typename OIT>
    void operator()(IT first, IT last, OIT out) const
    {
      impl_::template apply(first, last, out);
    }
  };

}

#if (defined(HRTREE_HAS_AVX) || defined(HRTREE_HAS_SSE2))
  #include <hrtree/arch/simd/mbr_build_policy_impl.hpp>
#endif
  

#endif
