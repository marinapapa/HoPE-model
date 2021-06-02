// hrtree/adapt_mbr.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ADAPT_MBR_HPP_INCLUDED
#define HRTREE_ADAPT_MBR_HPP_INCLUDED

#include <hrtree/config.hpp>
#include <hrtree/adapt_point.hpp>


#define HRTREE_ADAPT_MBR(Mbr, Point, MinAccess, MaxAccess) \
namespace hrtree { namespace traits { \
  template <> struct point_type<Mbr> { \
    typedef Point type; \
  }; \
  template <> struct mbr_access<Mbr> \
  { \
    static inline const Point& get_min(const Mbr& mbr) { return MinAccess; } \
    static inline const Point& get_max(const Mbr& mbr) { return MaxAccess; } \
    static inline Point& get_min(Mbr& mbr) { return MinAccess; } \
    static inline Point& get_max(Mbr& mbr) { return MaxAccess; } \
  }; \
}}


#define HRTREE_ADAPT_MBR_MEMBERS(Mbr, Point, PMin, PMax) \
  HRTREE_ADAPT_MBR(Mbr, Point, mbr. PMin, mbr. PMax)


#define HRTREE_ADAPT_MBR_FUNCTIONS(Mbr, Point, MinFun, MaxFun) \
  HRTREE_ADAPT_MBR(Mbr, Point, MinFun(mbr), MaxFun(mbr))


#define HRTREE_ADAPT_MBR_META(Mbr, Point, MinMeta, MaxMeta) \
  HRTREE_ADAPT_MBR(Mbr, Point, MinMeta::template apply<0>(mbr), MaxMeta::template apply<1>(mbr))


namespace hrtree { 
  
  namespace traits {

    template <typename Mbr>
    struct point_type;

    template <typename Mbr>
    struct mbr_access;

    template <size_t P>
    struct get_aux;

    template <>
    struct get_aux<0>
    {
      template <typename Mbr>
      static inline const typename point_type<Mbr>::type& apply(const Mbr& mbr)
      {
        return mbr_access<Mbr>::get_min(mbr);
      }

      template <typename Mbr>
      static inline typename point_type<Mbr>::type& apply(Mbr& mbr)
      {
        return mbr_access<Mbr>::get_min(mbr);
      }
    };

    template <>
    struct get_aux<1>
    {
      template <typename Mbr>
      static inline const typename point_type<Mbr>::type& apply(const Mbr& mbr)
      {
        return mbr_access<Mbr>::get_max(mbr);
      }

      template <typename Mbr>
      static inline typename point_type<Mbr>::type& apply(Mbr& mbr)
      {
        return mbr_access<Mbr>::get_max(mbr);
      }
    };

    template <size_t P, typename Mbr>
    inline const typename point_type<Mbr>::type& get(const Mbr& mbr)
    {
      static_assert(P <= 1, "Point dimension <= 1");
      return get_aux<P>::template apply(mbr);
    }

    template <size_t P, typename Mbr>
    inline typename point_type<Mbr>::type& get(Mbr& mbr)
    {
      static_assert(P <= 1, "Point dimension <= 1");
      return get_aux<P>::template apply(mbr);
    }

    template <size_t P, typename Mbr>
    inline const typename point_scalar<typename point_type<Mbr>::type>::type* get_ptr(const Mbr& mbr)
    {
      static_assert(P <= 1, "Point dimension <= 1");
      return point_access<typename point_type<Mbr>::type>::ptr(get_aux<P>::template apply(mbr));
    }

    template <size_t P, typename Mbr>
    inline typename point_scalar<typename point_type<Mbr>::type>::type* get_ptr(Mbr& mbr)
    {
      static_assert(P <= 1, "Point dimension <= 1");
      return point_access<typename point_type<Mbr>::type>::ptr(get_aux<P>::template apply(mbr));
    }

  }

}


#endif
