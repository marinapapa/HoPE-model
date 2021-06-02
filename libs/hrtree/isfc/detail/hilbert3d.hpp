// hrtree/isfc/detail/hilbert3d.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_DETAIL_HILBERT3D_HPP_INCLUDED
#define HRTREE_ISFC_DETAIL_HILBERT3D_HPP_INCLUDED

#include <hrtree/isfc/detail/fsm.hpp>


namespace hrtree { namespace detail {

namespace {

  struct hilbert3d_tab
  {
    static const unsigned char derived_key[12][8];
    static const unsigned char next_state[12][8];
  };

  const unsigned char hilbert3d_tab::derived_key[12][8] =
  {
    {0,1,3,2,7,6,4,5},
    {0,7,1,6,3,4,2,5},
    {0,3,7,4,1,2,6,5},
    {2,3,1,0,5,4,6,7},
    {4,3,5,2,7,0,6,1},
    {6,5,1,2,7,4,0,3},
    {4,7,3,0,5,6,2,1},
    {6,7,5,4,1,0,2,3},
    {2,5,3,4,1,6,0,7},
    {2,1,5,6,3,0,4,7},
    {4,5,7,6,3,2,0,1},
    {6,1,7,0,5,2,4,3}
  };

  const unsigned char hilbert3d_tab::next_state[12][8] =
  {
    {1,2,3,2,4,5,3,5},
    {2,6,0,7,8,8,0,7},
    {0,9,10,9,1,1,11,11},
    {6,0,6,11,9,0,9,8},
    {11,11,0,7,5,9,0,7},
    {4,4,8,8,0,6,10,6},
    {5,7,5,3,1,1,11,11},
    {6,1,6,10,9,4,9,10},
    {10,3,1,1,10,3,5,9},
    {4,4,8,8,2,7,2,3},
    {7,2,11,2,7,5,8,5},
    {10,3,2,6,10,3,4,4}
  };

} // anonymous namespace


  typedef ::hrtree::detail::table_fsm< hilbert3d_tab > hilbert3d_fsm;

} }


#endif
