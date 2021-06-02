// hrtree/isfc/detail/gray2d.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_DETAIL_GRAY2D_HPP_INCLUDED
#define HRTREE_ISFC_DETAIL_GRAY2D_HPP_INCLUDED

#include <hrtree/isfc/detail/fsm.hpp>


namespace hrtree { namespace detail {

namespace {

  struct gray2d_tab
  {
    static const unsigned char derived_key[2][4];
    static const unsigned char next_state[2][4];
  };

  const unsigned char gray2d_tab::derived_key[2][4] =
  {
    {0,3,1,2},
    {2,3,1,0}
  };

  const unsigned char gray2d_tab::next_state[2][4] =
  {
    {0,1,0,1},
    {1,0,1,0}
  };

} // anonymous namespace


  typedef ::hrtree::detail::table_fsm< gray2d_tab > gray2d_fsm;


} }


#endif
