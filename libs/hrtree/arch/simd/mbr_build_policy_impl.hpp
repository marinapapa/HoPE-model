// hrtree/arch/simd/mbr_build_policy_impl.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ARCH_SIMD_MBR_BUILD_POLICY_IMPL_HPP_INCLUDED
#define HRTREE_ARCH_SIMD_MBR_BUILD_POLICY_IMPL_HPP_INCLUDED

#include <hrtree/arch/simd/simd_aux.hpp>
#include <hrtree/mbr_build_policy.hpp>


namespace hrtree { namespace detail {


  template <typename simd_vec, typename Mbr>
  struct mbr_build_policy_impl
  {
    typedef traits::mbr_access<Mbr> pa;

    template <typename IT, typename OIT>
    static void apply(IT first, IT last, OIT lhs)
    {
      simd_vec lo = simd::load(pa::get_min(*lhs));
      simd_vec hi = simd::load(pa::get_max(*lhs));
      for (; first != last; ++first)
      {
        lo = simd::min(lo, simd::load(pa::get_min(*first)));
        hi = simd::max(hi, simd::load(pa::get_max(*first)));
      }
      simd::store(pa::get_min(*lhs), lo);
      simd::store(pa::get_max(*lhs), hi);
     }
  };


}}


#endif
