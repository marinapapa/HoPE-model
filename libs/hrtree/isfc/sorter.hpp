// hrtree/isfc/sorter.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_ISFC_SORTER_HPP
#define HRTREE_ISFC_SORTER_HPP

#include <utility>
#include <hrtree/config.hpp>
#include <hrtree/util/radix_sort.hpp>
#include <hrtree/util/parallel_radix_sort.hpp>
#include <hrtree/util/parallel_quick_sort.hpp>
#include <hrtree/detail/aligned_buffer.hpp>


namespace isfc {


  namespace detail {

    template <typename Key>
    struct select_key : std::unary_function<const std::pair<Key, unsigned>&, const Key& >
    {
      static const int size = Key::key_bytes;
      static const bool is_signed = false;
      const Key& operator()(const std::pair<Key, unsigned>& x) const { return x.first; }
    };


    template <typename Key>
    struct cmp_token_key
    {
      bool operator()(const std::pair<Key,unsigned>& a, const std::pair<Key,unsigned>& b) const
      {
        return a.first < b.first;
      }
    };


    template <typename IIt, typename OIt, typename KEYGEN>
    inline void gen_key_token(IIt src, OIt dst, const int N, const KEYGEN& keygen)
    {
      const int numt = hrtree_max_num_threads();
#     pragma omp parallel for firstprivate(src, dst) schedule(static) num_threads(numt)
      for (int i=0; i<N; ++i)
      {
        (dst + i)->first = keygen(*(src + i));
        (dst + i)->second = (unsigned)i;
      }
    }

  }


  template <
    typename Key, 
    typename A = /*::hrtree::detail::aligned_*/std::allocator<std::pair<Key,unsigned> >
  >
  class hilbert_radix_sort
  {
    typedef std::pair<Key, unsigned> elem;
    typedef ::hrtree::detail::aligned_buffer<elem, A> buffer;

  public:
    template <typename RaIt, typename KEYGEN>
    void operator()(RaIt first, RaIt last, const KEYGEN& keygen)
    {
      const size_t N = std::distance(first, last);
      buf0_.resize(N);
      buf1_.resize(N);
      detail::gen_key_token(first, buf0_.begin(), (int)N, keygen);    
      if (::hrtree::parallel_radix_sort(buf0_.begin(), buf0_.end(), buf1_.begin(), detail::select_key<Key>()))
        buf0_.swap(buf1_);
    }

  private:
    buffer buf0_;
    buffer buf1_;
  };


}


#endif
