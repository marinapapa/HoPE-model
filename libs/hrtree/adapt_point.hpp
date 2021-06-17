// hrtree/adapt_point.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ADAPT_POINT_HPP_INCLUDED
#define HRTREE_ADAPT_POINT_HPP_INCLUDED

#include <type_traits>
#include <hrtree/config.hpp>

namespace hrtree {

  namespace traits {

    template <typename Point>
    struct point_dim;

    template <typename Point>
    struct point_scalar;

    template <typename Point>
    struct point_access;

  }

}

#define HRTREE_ADAPT_POINT(Point, Scalar, Dim, Access) \
namespace hrtree { namespace traits { \
  template <> struct point_dim<Point> : std::integral_constant<int, Dim> {}; \
  template <> struct point_scalar<Point> { typedef Scalar type; }; \
  template <> struct point_access<Point> \
  { \
    static inline const Scalar* ptr(const Point& p) { return Access; } \
    static inline Scalar* ptr(Point& p) { return Access; } \
  }; \
}}


#define HRTREE_ADAPT_POINT_MEMBER(Point, Scalar, Dim, Member) \
  HRTREE_ADAPT_POINT(Point, Scalar, Dim, p.Member)


#define HRTREE_ADAPT_POINT_FUNCTION(Point, Scalar, Dim, Fun) \
  HRTREE_ADAPT_POINT(Point, Scalar, Dim, Fun(p))


#define HRTREE_ADAPT_POINT_META(Point, Scalar, Dim, Meta) \
  HRTREE_ADAPT_POINT(Point, Scalar, Dim, Meta::template apply(p))


#endif
