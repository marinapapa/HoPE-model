// hrtree/isfc/key_gen.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_KEY_GEN_HPP_INCLUDED
#define HRTREE_ISFC_KEY_GEN_HPP_INCLUDED

#include <hrtree/config.hpp>
#include <hrtree/adapt_point.hpp>
#include <hrtree/arch/select.hpp>


namespace hrtree {

  namespace detail {

    template <typename simd_vec, typename Key, typename Point>
    class key_gen_01_impl;

  
    template <typename simd_vec, typename Key, typename Point>
    class key_gen_impl;


    template < typename Key, typename Point >
    class key_gen_01_impl< unsupported, Key, Point >
    {
      typedef traits::point_access<Point> pa;
      typedef typename traits::point_scalar<Point>::type scalar;

    public:
      key_gen_01_impl() {}

      Key apply(const Point& p) const
      {
        typename Key::arg_type norm[Key::dim];
        const scalar scale(Key::max_arg);
        for (int d=0; d < Key::dim; ++d)
        {
          norm[d] = static_cast<typename Key::arg_type>(*(pa::ptr(p) + d) * scale);
        }
        return Key(norm);
      }
    };

    template < typename Key, typename Point >
    class key_gen_impl< unsupported, Key, Point >
    {
      typedef traits::point_access<Point> pa;
      typedef typename traits::point_scalar<Point>::type scalar;

    public:
      key_gen_impl()
      {
        for (int d=0; d < Key::dim; ++d)
        {
          org_[d] = scalar(0);
          scale_[d] = static_cast<scalar>(Key::max_arg);
        }
      }

      key_gen_impl(const Point& lo, const Point& hi)
      {
        for (int d=0; d < Key::dim; ++d)
        {
          org_[d] = lo[d];
          scale_[d] = static_cast<scalar>(Key::max_arg) / (*(pa::ptr(hi) + d) - *(pa::ptr(lo) + d));
        }
      }

      Key apply(const Point& p) const
      {
        typename Key::arg_type norm[Key::dim];
        for (int d=0; d < Key::dim; ++d)
        {
          norm[d] = static_cast<typename Key::arg_type>((*(pa::ptr(p) + d) - org_[d]) * scale_[d]);
        }
        return Key(norm);
      }

    private:
      scalar org_[Key::dim];
      scalar scale_[Key::dim];
    };

  }

  template <typename Key, typename Point>
  class key_gen_01
  {
    typedef typename simd_type<Point>::type simd_vec;
    detail::key_gen_01_impl<simd_vec, Key, Point> impl_;

  public:
    key_gen_01() : impl_() {}
    Key operator()(const Point& p) const { return impl_.apply(p); }
  };


  template <typename Key, typename Point>
  class key_gen
  {
    static_assert(Key::dim == traits::point_dim<Point>::value, "Key/Point dimension mismatch");
    typedef typename simd_type<Point>::type simd_vec;
    detail::key_gen_impl<simd_vec, Key, Point> impl_;

  public:
    key_gen(const Point& lo, const Point& hi) : impl_(lo, hi) {}
    Key operator()(const Point& p) const { return impl_.apply(p); }
  };

}


#if (defined (HRTREE_HAS_AVX)  || defined (HRTREE_HAS_SSE2))
  #include <hrtree/arch/simd/key_gen_impl.hpp>
#endif


#endif
