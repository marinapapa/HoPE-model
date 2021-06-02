// hrtree/isfc/detail/fsm.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_DETAIL_FSM_HPP_INCLUDED
#define HRTREE_ISFC_DETAIL_FSM_HPP_INCLUDED


namespace hrtree { namespace detail {


  struct nop_fsm {
    unsigned operator()(unsigned n_point) const { return n_point; }
  };


  template <typename BASE_>
  class table_fsm : private BASE_
  {
    unsigned current_state;

  public:
    table_fsm(): current_state(0) {}

    unsigned operator()(unsigned n_point)
    {
      unsigned ret = BASE_::derived_key[current_state][n_point];
      current_state = BASE_::next_state[current_state][n_point];
      return ret;
    }
  };


} }


#endif
