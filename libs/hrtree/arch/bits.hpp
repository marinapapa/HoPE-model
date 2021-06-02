// isfc/detail/bits.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef ISFC_DETAIL_BITS_HPP_INCLUDED
#define ISFC_DETAIL_BITS_HPP_INCLUDED


namespace hrtree { namespace detail {


  template <bool>
  struct set_n_point_aux    // true: first touch of x
  {
    template<int S_, typename W>
    static void value(W& x, W n_point) { x = (n_point << S_); }
  };

  
  template <>
  struct set_n_point_aux<false>
  {
    template<int S_, typename W>
    static void value(W& x, W n_point) { x |= (n_point << S_); }
  };

  
  template <bool>
  struct set_n_point_carry    // true: we must handle carry
  {
    template<int S_, typename W>
    static void value(W& x, W n_point) { x |= (n_point >> S_); }
  };

  
  template <>
  struct set_n_point_carry<false>
  {
    template<int S_, typename W>
    static void value(W& x, W n_point) {}
  };

  
  template <typename K_, int ITER_>
  struct set_n_point_consts
  {
    static const int whichword = ((K_::dim * ITER_) / K_::word_bits);
    static const int shift = ((K_::dim * ITER_) - (whichword * K_::word_bits));
    static const int carry_bits = ((shift + K_::dim) - K_::word_bits);
    static const bool has_carry = (carry_bits > 0);
    static const int non_carry_bits = (K_::dim - carry_bits);
    static const bool first_touch = (carry_bits >= 0) || (ITER_ == (K_::order-1));
  };

  
  template <typename K_>
  struct set_n_point
  {
    template <int ITER_>
    static void value(K_& key, typename K_::word_type n_point) 
    {
      set_n_point_aux< set_n_point_consts<K_,ITER_>::first_touch >::template
        value< set_n_point_consts<K_,ITER_>::shift >(key.val_[set_n_point_consts<K_,ITER_>::whichword], n_point);
      set_n_point_carry< set_n_point_consts<K_,ITER_>::has_carry >::template
        value< set_n_point_consts<K_,ITER_>::non_carry_bits >(key.val_[set_n_point_consts<K_,ITER_>::whichword + 1], n_point);
    }
  };


  template <int DIM, typename ARG_, int REP_>
  struct do_bitpattern
  {
    static const int NEXTREP = (REP_-1);
    static const ARG_ value = (do_bitpattern< DIM, ARG_, NEXTREP >::value << DIM) | ARG_(1);
  };

  
  template <int DIM, typename ARG_>
  struct do_bitpattern<DIM, ARG_, 1>
  {
    static const ARG_ value = ARG_(1);
  };

  
  // Returns the repetitive bit pattern "DIM_-1 zeros followed by 1".
  // DIM_ == 2: ...01010101.
  // DIM_ == 3: ...001001001.
  //
  template <int DIM, typename ARG_>
  struct bitpattern
  {
    static const int REP = (std::numeric_limits<ARG_>::digits / DIM);
    static const ARG_ value = (do_bitpattern< DIM, ARG_, REP >::value);
  };


  template <typename ARG_, int S_, bool>
  struct do_signed_shift {
    static ARG_ value(ARG_ x) {
      return x << S_;
    }
  };

  
  template <typename ARG_, int S_>
  struct do_signed_shift<ARG_, S_, false> {
    static ARG_ value(ARG_ x) {
      return x >> -S_;
    }
  };


  // Returns (x << S_) if S_ is positive, (x >> -S_) else.
  //
  template <typename ARG_, int S_>
  struct signed_shift  {
    static const bool SSHL = (S_ > 0);

    static ARG_ value(ARG_ x) {
      return do_signed_shift< ARG_, S_, SSHL >::value(x);
    }
  };


  // Interleaves the bits from in[0] to in[DIM_-1], so that the
  // result contains the bit-tuples ..., p(OUT_+2*DIM),p(OUT_+DIM_),p(OUT_).
  //
  // example (DIM_ == 2, OUT_ == 1):
  // in[0] = x  x  x  x  x
  //          4  3  2  1  0
  // in[1] = y  y  y  y  y
  //          4  3  2  1  0
  // result <- y x  y x
  //            3 3  1 1
  //
  // partial_interleave_inner iterates over in[DIM_] while
  // partial_interleave iterates over out[DIM_].
  //
  template <int DIM, typename ARG_, int OUT_, int IN_>
  struct partial_interleave_inner
  {
    static const int NEXTIN = (IN_-1);
    static const int SSHIFT = (DIM-1-IN_-OUT_);
    static const int INCOORD = (DIM-1-IN_);

    static ARG_ value(const ARG_ *const in)
    {
      return partial_interleave_inner< DIM,ARG_,OUT_,NEXTIN >::value(in) |
        signed_shift<ARG_, SSHIFT >::value(
          in[INCOORD] & (bitpattern<DIM,ARG_>::value << OUT_)
        );
    }
  };

  template <int DIM, typename ARG_, int OUT_>
  struct partial_interleave_inner<DIM, ARG_, OUT_, 0>
  {
    static const int SSHIFT = (DIM-1-0-OUT_);
    static const int INCOORD = (DIM-1);

    static ARG_ value(const ARG_ *const in)
    {
      return signed_shift< ARG_, SSHIFT >::value(
          in[INCOORD] & (bitpattern<DIM,ARG_>::value << OUT_)
        );
    }
  };


  // Interleaves the bits from in[0] to in[DIM_-1], so that
  // out[i] contains the bit-tuples ..., p(i+2*DIM_), p(i+DIM_), p(i).
  //
  // example (DIM_ == 2):
  // in[0] = x  x  x  x  x
  //          4  3  2  1  0
  // in[1] = y  y  y  y  y
  //          4  3  2  1  0
  // out[0] <- y x  y x  y x    by partial_interleave_inner< OUT_ == 0 >
  //            4 4  2 2  0 0
  // out[1] <- y x  y x      by partial_interleave_inner< OUT_ == 1 >
  //            3 3  1 1
  //
  // partial_interleave_inner iterates over in[DIM_] while
  // partial_interleave iterates over out[DIM_].
  //
  template <typename KEY, int OUT_>
  struct partial_interleave
  {
    static const int NEXTOUT = (OUT_ -1);
    static const int FIRSTIN = (KEY::dim - 1);

    static void value(const typename KEY::arg_type *const in,
      typename KEY::arg_type *const out)
    {
      // assert( in[OUT_] <= ISFC_::max_arg );
      partial_interleave< KEY, NEXTOUT >::value(in, out);
      out[OUT_] = partial_interleave_inner<
        KEY::dim,
        typename KEY::arg_type,
        OUT_,
        FIRSTIN
      >::value(in);
    }
  };


  template <typename KEY>
  struct partial_interleave<KEY,0>
  {
    static const int FIRSTIN = (KEY::dim - 1);

    static void value(const typename KEY::arg_type *const in,
      typename KEY::arg_type *const out)
    {
      // assert( in[0] <= ISFC_::max_arg );
      out[0] = partial_interleave_inner<
        KEY::dim,
        typename KEY::arg_type,
        0,
        FIRSTIN
      >::value(in);
    }
  };


  // Returns the ITER_th n_point in bci
  //
  struct raw_n_point
  {
    template <int DIM, int ITER_, typename ARG_>
    static unsigned value(const ARG_ *const bci)
    {
      return (bci[(ITER_%DIM)] >> ((ITER_/DIM)*DIM)) & ((1u<<DIM)-1u);
    }
  };


  // Constructs the the final key from the partially interleaved bit tuples
  // in bci. Each bit tuple (n_point) is transformed by the finite state machine
  // in fsm before collected into the key k.
  //
  template <int ITER_>
  struct isfc_aux
  {
    static const int NEXTITER = (ITER_-1);

    template<int DIM, typename KEY, typename FSM, typename ARG_>
    static void value(KEY & k, FSM & fsm, const ARG_ *const bci)
    {
      typename KEY::word_type n_point = fsm( raw_n_point::template value<DIM,ITER_,ARG_>(bci) );
      set_n_point<KEY>::template value<ITER_>(k, n_point);
      isfc_aux< NEXTITER >::template value<DIM, KEY, FSM, ARG_>(k, fsm, bci);
    }
  };


  template <>
  struct isfc_aux<0>
  {
    template<int DIM, typename KEY, typename FSM, typename ARG_>
    static void value(KEY & k, FSM & fsm, const ARG_ *const bci)
    {
      typename KEY::word_type n_point = fsm( bci[0] & ((1u<<DIM)-1u) );
      set_n_point<KEY>::template value<0>(k, n_point);
    }
  };


} }


#endif
