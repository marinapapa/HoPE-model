// hrtree/arch/simd/key_gen_impl.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef ISFC_ARCH_SIMD_KEY_GEN_IMPL_HPP_INCLUDED
#define ISFC_ARCH_SIMD_KEY_GEN_IMPL_HPP_INCLUDED

#include <type_traits>
#include <hrtree/isfc/key_gen.hpp>
#include <hrtree/arch/simd/simd_aux.hpp>


namespace hrtree { namespace detail {
  

  template <typename simd_vec, typename Key, typename Point>
  class key_gen_01_impl
  {
    simd_vec scale_;

  public:
    key_gen_01_impl() : scale_(simd::set<simd_vec>::template val(Key::max_arg)) 
    {}

    Key apply(simd_vec p) const 
    { 
      return Key(simd::cvtt_epi32(simd::mul(p, scale_))); 
    }

    Key apply(const Point& p) const
    {
      return apply(simd::load(p));
    }
  };


  template <typename simd_vec, typename Key>
  class key_gen_01_impl<simd_vec, Key, simd_vec>
  {
    simd_vec scale_;

  public:
    key_gen_01_impl() : scale_(simd::set<simd_vec>::template val(Key::max_arg))
    {}

    Key apply(simd_vec p) const
    {
      return Key(simd::cvtt_epi32(simd::mul(p, scale_)));
    }
  };


  template <typename simd_vec, typename Key, typename Point>
  class key_gen_impl
  {
    simd_vec org_;
    simd_vec scale_;

  public:
    key_gen_impl()
    : org_( simd::set<simd_vec>::zero() ), 
      scale_( simd::set<simd_vec>::template val(Key::max_arg) )
    {
    }

    key_gen_impl(const Point& lo, const Point& hi)
    {
      org_ = simd::load(lo);
      scale_ = simd::div(simd::set<simd_vec>::template val(Key::max_arg), simd::sub(simd::load(hi), org_));
    }

    Key apply(simd_vec args) const
    {
      return Key(simd::cvtt_epi32( simd::mul(simd::sub(args, org_), scale_) ));
    }

    Key apply(const Point& p) const
    {
      return apply(simd::load(p));
    }
  };


  template <typename simd_vec, typename Key>
  class key_gen_impl<simd_vec, Key, simd_vec>
  {
    simd_vec org_;
    simd_vec scale_;

  public:
    key_gen_impl()
      : org_(simd::set<simd_vec>::zero()),
      scale_(simd::set<simd_vec>::template val(Key::max_arg))
    {
    }

    key_gen_impl(simd_vec lo, simd_vec hi)
    {
      org_ = lo;
      scale_ = simd::div(simd::set<simd_vec>::template val(Key::max_arg), simd::sub(hi, org_));
    }

    Key apply(simd_vec args) const
    {
      return Key(simd::cvtt_epi32(simd::mul(simd::sub(args, org_), scale_)));
    }
  };


}
}


#endif
