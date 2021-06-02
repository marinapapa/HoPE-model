// hrtree/isfc/detail/gray3d.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_DETAIL_GRAY3D_HPP_INCLUDED
#define HRTREE_ISFC_DETAIL_GRAY3D_HPP_INCLUDED

#include <hrtree/isfc/detail/fsm.hpp>


namespace hrtree { namespace detail {

namespace {

  struct gray3d_tab
  {
    static const unsigned char derived_key[4][8];
    static const unsigned char next_state[4][8];
  };

  const unsigned char gray3d_tab::derived_key[4][8] =
  {
    {0,1,3,2,6,7,5,4},
    {5,4,6,7,3,2,0,1},
    {3,2,0,1,5,4,6,7},
    {6,7,5,4,0,1,3,2}
  };

  const unsigned char gray3d_tab::next_state[4][8] =
  {
    {0,1,2,3,3,2,1,0},
    {1,0,3,2,2,3,0,1},
    {2,3,0,1,1,0,3,2},
    {3,2,1,0,0,1,2,3}
  };

} // anonymous namespace


  typedef ::hrtree::detail::table_fsm< gray3d_tab > gray3d_fsm;

}}


#endif
