// hrtree/isfc/morton.hpp header file
//
// FSM for Z-Order (Peano) curve based on morton code.
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_MORTON_HPP_INCLUDED
#define HRTREE_ISFC_MORTON_HPP_INCLUDED


#include <hrtree/isfc/key.hpp>
#include <hrtree/isfc/detail/fsm.hpp>


namespace hrtree {


  template <int DIM, int ORDER>
  struct morton {
    typedef key<DIM, ORDER, detail::nop_fsm> type;
  };


}

#endif
