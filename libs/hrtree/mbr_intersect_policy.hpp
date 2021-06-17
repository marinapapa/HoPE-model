// hrtree/mbr_intersect_policy.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_MBR_INTERSECT_POLICY_HPP_INCLUDED
#define HRTREE_MBR_INTERSECT_POLICY_HPP_INCLUDED

#include <cassert>
#include <hrtree/adapt_mbr.hpp>
#include <hrtree/arch/select.hpp>


namespace hrtree {

  namespace detail {

    template <typename simd_vec, typename Mbr>
    struct mbr_intersect_policy_impl;


    template <typename Mbr>
    struct mbr_intersect_policy_impl<unsupported, Mbr>
    {
      typedef typename traits::point_type<Mbr>::type point;
      typedef typename traits::point_scalar<point>::type scalar;

      mbr_intersect_policy_impl(const Mbr& pivot): pivot_(pivot) {}

      bool apply(const Mbr& rhs) const
      {
        for (std::size_t i = 0; i < traits::point_dim<point>::value; ++i)
        {
          if ((*(traits::get_ptr<1>(pivot_) + i) < *(traits::get_ptr<0>(rhs) + i)) ||
               (*(traits::get_ptr<0>(pivot_) + i) > *(traits::get_ptr<1>(rhs) + i))) return false;
        }
        return true;
      }

      bool apply(unsupported, unsupported) const
      {
        assert( !"unsupported simd_type");
        return true;
      }

      const Mbr pivot_;
    };

  }


  template <typename Mbr>
  class mbr_intersect_policy
  {
    typedef typename traits::point_type<Mbr>::type point;
    typedef typename simd_type<point>::type simd_vec;

  public:
    mbr_intersect_policy(const Mbr& pivot) : impl_(pivot) {}

    bool operator()(const Mbr& rhs) const
    {
      return impl_.apply(rhs);
    }

#if (defined(HRTREE_HAS_AVX) || defined(HRTREE_HAS_SSE2))
    bool operator()(simd_vec lo, simd_vec hi) const
    {
      return impl_.apply(lo, hi);
    }
#endif

  private:
    detail::mbr_intersect_policy_impl<simd_vec, Mbr> impl_;
  };

}


#if (defined(HRTREE_HAS_AVX) || defined(HRTREE_HAS_SSE2))
  #include <hrtree/arch/simd/mbr_intersect_policy_impl.hpp>
#endif
  

#endif
