// hrtree/isfc/gray.hpp header file
//
// FSM for Gray curve based on Gray codes.
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_GRAY_HPP_INCLUDED
#define HRTREE_ISFC_GRAY_HPP_INCLUDED


#include <hrtree/isfc/key.hpp>
#include <hrtree/isfc/detail/gray2d.hpp>
#include <hrtree/isfc/detail/gray3d.hpp>


namespace hrtree {


  template <int DIM, int ORDER>
  struct gray {};

  template <int ORDER> struct gray<2,ORDER> { 
    typedef key<2, ORDER, detail::gray2d_fsm> type;
  };

  template <int ORDER> struct gray<3,ORDER> { 
    typedef key<3, ORDER, detail::gray3d_fsm> type;
  };


}

#endif

