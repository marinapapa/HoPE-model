// hrtree/isfc/hilbert.hpp header file
//
// FSM for Hilbert curve based on Hilbert values.
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_HILBERT_HPP_INCLUDED
#define HRTREE_ISFC_HILBERT_HPP_INCLUDED


#include <hrtree/isfc/key.hpp>
#include <hrtree/isfc/detail/hilbert2d.hpp>
#include <hrtree/isfc/detail/hilbert3d.hpp>
#include <hrtree/isfc/detail/hilbert4d.hpp>


namespace hrtree {


  template <int DIM, int ORDER>
  struct hilbert {};

  template <int ORDER> struct hilbert<2,ORDER> { 
    typedef key<2, ORDER, detail::hilbert2d_fsm> type;
  };

  template <int ORDER> struct hilbert<3,ORDER> { 
    typedef key<3, ORDER, detail::hilbert3d_fsm> type;
  };

  template <int ORDER> struct hilbert<4,ORDER> { 
    typedef key<4, ORDER, detail::hilbert4d_fsm> type;
  };


}

#endif

