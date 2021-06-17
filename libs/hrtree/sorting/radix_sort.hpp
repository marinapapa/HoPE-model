// hrtree/sorting/radix_sort.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_RADIX_SORT_HPP
#define HRTREE_SORTING_RADIX_SORT_HPP

#include <limits>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <memory.h>
#include <type_traits>
#include <hrtree/config.hpp>
#include <hrtree/sorting/insertion_sort.hpp>
#include <hrtree/zip/zip.hpp>


namespace hrtree { namespace sorting { namespace detail {

  
  template <typename T, typename Enable = void>
  struct default_converter 
  {
    static const int key_bytes = sizeof(T);
    const std::uint8_t* operator()(const T& x) const { return (const std::uint8_t*)&*std::addressof(x); }
  };


  template <typename T>
  struct default_converter<T,  typename std::enable_if< T::is_isfc_key::value >::type > 
  {
    static const int key_bytes = T::key_bytes;
    const std::uint8_t* operator()(const T& x) const { return x.data(); }
  };


  template <typename CONV>
  struct conv_less_cmp
  {
    conv_less_cmp(const CONV& conv, int byte): byte_(byte), conv_(conv) {}
    template <typename T>
    bool operator()(const T& x, const T& y) const
    {
      const std::uint8_t* px = conv_(x);
      const std::uint8_t* py = conv_(y);
      for (int b = byte_; b > 0; --b)     
        if (*(px + b) != *(py + b)) 
          return *(px + b) < *(py + b);
      return *px < *py;
    }
    const int byte_;
    const CONV conv_;
  };


  template <typename ZIt, typename CONV>
  inline bool lsd_radix_sort_impl(ZIt src, ZIt buf, const int N, CONV conv, const int key_bytes)
  {
    int bins[UINT8_MAX+1];
    bool swaped = false;
    for (int byte = 0; byte < key_bytes; ++byte)
    {
      memset(bins, 0, (UINT8_MAX+1)*sizeof(int));
      auto src0(zip::head(src));
      for (int i = 0; i < N; ++i)
      {
        std::uint8_t key = *(conv(*(src0 + i)) + byte);
        ++bins[key];
      }
      
      // Reduce prefix table
      int cumsum = 0;
      for (int b = 0; b <= UINT8_MAX; ++b)
      {
        const int count = bins[b];
        if (count == N) goto skip;
        bins[b] = cumsum;
        cumsum += count;
      }

      // Scatter
      for (int i = 0; i<N; ++i)
      {
        std::uint8_t key = *(conv(*(src0 + i)) + byte);
        zip::iter_move(src, buf, i, bins[key]++);
      }
      std::swap(src, buf);
      swaped = !swaped;
skip:  
      ;
    }
    return swaped;
  }


  template <typename ZIt, typename CONV>
  inline int inplace_msd_radix_sort_prepare(ZIt src, const int N, CONV conv, int byte, int bins[UINT8_MAX+1], int ends[UINT8_MAX+1])
  {
    auto src0(zip::head(src));
    for (; byte >= 0; --byte)
    {
      memset(bins, 0, (UINT8_MAX+1)*sizeof(int));
      for (int i=0; i<N; ++i)
      {
        std::uint8_t key = *(conv(*(src0 + i)) + byte);
        ++bins[key];
      }
      int cumsum = 0;
      int j = 0;
      for (int b=0; b<=UINT8_MAX; ++b) 
      {
        int count = bins[b];
        if (count == N) goto skip;
        bins[b] = ends[b] = cumsum;
        cumsum += count;
      }
      for (int i=0; i<N; )
      {
        std::uint8_t key = *(conv(*(src0 + i)) + byte);
        if (ends[key] != i)
        {
          // not in place. swap with element from its bin
          std::iter_swap(src + i, src + ends[key]++);
        }
        else
        {
          ++ends[key];    // in place - skip
          ++i;            // next element
          while (i >= bins[j] && (j <= UINT8_MAX)) ++j;          // select bin of next element
          while (ends[j-1] == bins[j] && (j <= UINT8_MAX)) ++j;  // skip handled (empty) bins
          if (i < ends[j-1]) i = ends[j-1];                     // skip elements in handled bins 
        }
      }
      break;
skip :
      ;
    }
    return byte;
  }


  template <typename ZIt, typename CONV>
  inline void inplace_msd_radix_sort_impl(ZIt src, const int N, CONV conv, int byte)
  {
    int bins[UINT8_MAX+1];
    int ends[UINT8_MAX+1];

    byte = inplace_msd_radix_sort_prepare(src, N, conv, byte, bins, ends);
    if (byte > 0)
    {
      for (int b = 0; b <= UINT8_MAX; ++b)
      {
        int n = ends[b] - bins[b];
        if (1 < n)
        {
          if (32 >= n) insertion_sort_impl(src + bins[b], n, conv_less_cmp<CONV>(conv, byte - 1));
          else inplace_msd_radix_sort_impl(src + bins[b], n, conv, byte - 1);
        }
      }
    }
  }


}  // namespace detail

  
template < typename ZIt, typename CONV >
inline bool radix_sort(ZIt first, ZIt last, ZIt buf, CONV conv)
{
  int N = int(last - first);
  return (1 < N) ? detail::lsd_radix_sort_impl(first, buf, N, conv, CONV::key_bytes) : false;
}


template < typename ZIt >
inline bool radix_sort(ZIt first, ZIt last, ZIt buf)
{
  typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
  return radix_sort(first, last, buf, detail::default_converter<T>());
}


template < typename ZIt, typename CONV >
inline void inplace_radix_sort(ZIt first, ZIt last, CONV conv)
{  
  int N = int(last - first);
  if (1 < N)
  {
    if (32 >= N) insertion_sort_impl(first, N, detail::conv_less_cmp<CONV>(conv, CONV::key_bytes-1));
    else detail::inplace_msd_radix_sort_impl(first, N, conv, CONV::key_bytes-1);
  }
}


template < typename ZIt >
inline void inplace_radix_sort(ZIt first, ZIt last)
{
  typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
  inplace_radix_sort(first, last, detail::default_converter<T>());
}


}

using sorting::inplace_radix_sort;
using sorting::radix_sort;

}

#endif
