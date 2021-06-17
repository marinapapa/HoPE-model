// hrtree/arch/simd/mbr_intersect_policy_impl.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ARCH_SIMD_MBR_INTERSECT_POLICY_IMPL_HPP_INCLUDED
#define HRTREE_ARCH_SIMD_MBR_INTERSECT_POLICY_IMPL_HPP_INCLUDED

#include <hrtree/mbr_intersect_policy.hpp>
#include <hrtree/arch/simd/simd_aux.hpp>


namespace hrtree { namespace detail {

  template <typename simd_vec, typename Mbr>
  struct mbr_intersect_policy_impl
  {
    static const int dim = traits::point_dim<typename traits::point_type<Mbr>::type>::value;

    typedef traits::mbr_access<Mbr> pa;

    simd_vec lo_;
    simd_vec hi_;

    mbr_intersect_policy_impl(const Mbr& pivot)
    {
      lo_ = simd::load(pa::get_min(pivot));
      hi_ = simd::load(pa::get_max(pivot));
    }

    bool apply(simd_vec lo, simd_vec hi) const
    {
      simd_vec cmp0 = simd::cmpge(hi_, lo);
      simd_vec cmp1 = simd::cmple(lo_, hi);
      return 0xF == simd::movemask(simd::band(cmp0, cmp1));
    }

    bool apply(const Mbr& rhs) const
    {
      simd_vec lo = simd::load(pa::get_min(rhs));
      simd_vec hi = simd::load(pa::get_max(rhs));
      return apply(lo, hi);
    }

  };


} }

#endif
