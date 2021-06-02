// hrtree/isfc/detail/hilbert2d.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_DETAIL_HILBERT2D_HPP_INCLUDED
#define HRTREE_ISFC_DETAIL_HILBERT2D_HPP_INCLUDED

#include <hrtree/isfc/detail/fsm.hpp>


namespace hrtree { namespace detail {

namespace {

  struct hilbert2d_tab
  {
    static const unsigned char derived_key[4][4];
    static const unsigned char next_state[4][4];
  };

  const unsigned char hilbert2d_tab::derived_key[4][4] =
  {
    {0,1,3,2},
    {0,3,1,2},
    {2,1,3,0},
    {2,3,1,0}
  };

  const unsigned char hilbert2d_tab::next_state[4][4] =
  {
    {1,0,2,0},
    {0,3,1,1},
    {2,2,0,3},
    {3,1,3,2}
  };

} // anonymous namespace


  typedef ::hrtree::detail::table_fsm< hilbert2d_tab > hilbert2d_fsm;

} }

#endif
