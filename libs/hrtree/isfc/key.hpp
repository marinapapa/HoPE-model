// hrtree/isfc/key.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_ISFC_KEY_HPP_INCLUDED
#define HRTREE_ISFC_KEY_HPP_INCLUDED

#include <limits.h>
#include <limits>
#include <cstdint>
#include <type_traits>
#include <hrtree/config.hpp>
#include <hrtree/arch/simd/simd_aux.hpp>
#include <hrtree/arch/bits.hpp>


namespace hrtree {
  
  namespace detail {

  
    template <int BITS>
    struct select_uint
    {
      typedef typename std::conditional <
        BITS <= 32,
        std::uint_fast32_t,
        std::uint_fast64_t
      > ::type type;

      static const type max = (BITS <= 32) ? UINT_FAST32_MAX : UINT_FAST64_MAX;
    };

  }


#if (defined (HRTREE_HAS_AVX) || defined (HRTREE_HAS_SSE2))

  namespace detail {

    template <int ITER>
    struct key_ctor_aux
    {
      static const int NEXTITER = (ITER-1);

      template<int DIM, typename Key, typename FSM>
      static void value(Key & k, FSM & fsm, __m128i args)
      {
        static const int mask = (0xF >> (4-DIM));
        typename Key::word_type n_point = fsm(_mm_movemask_ps(_mm_castsi128_ps(args)) & mask);
        set_n_point<Key>::template value<ITER>(k, n_point);
        key_ctor_aux< NEXTITER >::template value<DIM, Key, FSM>(k, fsm, _mm_slli_epi32(args, 1));
      }
    };

    template <>
    struct key_ctor_aux<0>
    {
      template<int DIM, typename Key, typename FSM>
      static void value(Key & k, FSM & fsm, __m128i args)
      {
        static const int mask = (0xF >> (4-DIM));
        typename Key::word_type n_point = fsm(_mm_movemask_ps(_mm_castsi128_ps(args)) & mask);
        set_n_point<Key>::template value<0>(k, n_point);
      }
    };

  }

#endif


  template<int DIM, int ORDER, typename FSM, typename WORD_TYPE = unsigned int>
  class key
  {
  public:
    static const int dim = DIM;
    static const int order = ORDER;
    static const int key_bits = (dim * order);
    typedef typename detail::select_uint<order>::type arg_type;
    typedef WORD_TYPE word_type;
    static const int arg_digits = std::numeric_limits<arg_type>::digits;
    static const arg_type max_arg = ( detail::select_uint<order>::max >> (arg_digits - order));
    static const int key_bytes = (key_bits + CHAR_BIT - 1) / CHAR_BIT;
    static const int key_words = (key_bytes + sizeof(word_type) - 1) / sizeof(word_type);
    static const int word_bits = std::numeric_limits<word_type>::digits;

    typedef std::true_type is_isfc_key;
    typedef FSM fsm_type;

    key() = default;

#if (defined (HRTREE_HAS_AVX) || defined (HRTREE_HAS_SSE2))

    explicit key(__m128i args)
    {
      static const int FIRSTITER = (order-1);
      fsm_type fsm;
      detail::key_ctor_aux<FIRSTITER>::template value<dim>(*this, fsm, _mm_slli_epi32(args, 32-order));
    }

#endif 

    explicit key(const arg_type* const args)
    {
      static const int FIRSTOUT = (DIM-1);
      static const int FIRSTITER = (ORDER-1);

      arg_type bci[DIM];
      fsm_type fsm;
      detail::partial_interleave< key, FIRSTOUT >::value(args, bci);
      detail::isfc_aux< FIRSTITER >::template value<DIM>(*this, fsm, bci);
    }

    word_type asWord() const
    {
      static_assert(key_words == 1, "hrtree::key: word_ype can't hold key");
      return val_[0];
    }

    const unsigned char* data() const { return (const unsigned char*)val_; }

    friend bool operator == (const key& a, const key& b)
    {  
      for (int d=0; d < key_words; ++d) 
        if (a.val_[d] != b.val_[d]) return false;
      return true;                          
    }

    friend bool operator < (const key& a, const key& b) 
    {  
      for (int d = key_words-1; d > 0; --d)     
        if (a.val_[d] != b.val_[d]) 
          return a.val_[d] < b.val_[d];
      return a.val_[0] < b.val_[0];
    }

  private:
    friend struct ::hrtree::detail::set_n_point< key<DIM, ORDER, FSM> >;
    word_type val_[key_words];
  };


  template<int DIM, int ORDER, typename FSM>
  inline bool operator != (const key<DIM,ORDER,FSM>& a, const key<DIM,ORDER,FSM>& b) 
  {
    return !(a == b);
  }


  template<int DIM, int ORDER, typename FSM>
  inline bool operator > (const key<DIM,ORDER,FSM>& a, const key<DIM,ORDER,FSM>& b) 
  {
    return b < a;
  }


  template<int DIM, int ORDER, typename FSM>
  inline bool operator <= (const key<DIM,ORDER,FSM>& a, const key<DIM,ORDER,FSM>& b) 
  {
    return !(b < a);
  }


  template<int DIM, int ORDER, typename FSM>
  inline bool operator >= (const key<DIM,ORDER,FSM>& a, const key<DIM,ORDER,FSM>& b) 
  {
    return !(a < b);
  }

}


#endif
