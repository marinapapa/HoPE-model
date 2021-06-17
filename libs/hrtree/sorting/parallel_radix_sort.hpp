// hrtree/sorting/parallel_quick_sort.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_PARALLEL_RADIX_SORT_HPP
#define HRTREE_SORTING_PARALLEL_RADIX_SORT_HPP

#include <algorithm>
#include <hrtree/config.hpp>
#include <hrtree/sorting/radix_sort.hpp>


namespace hrtree { namespace sorting { namespace detail {


  template <typename ZIt, typename CONV>
  inline bool parallel_radix_sort_impl(ZIt src, ZIt buf, const int N, CONV conv, const int key_bytes)
  {
    HRTREE_ALIGN_CACHELINE int Bins[HRTREE_OMP_MAX_THREADS][UINT8_MAX+1];
    int SingularBin[HRTREE_OMP_MAX_THREADS];
    bool Swaped = false;
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel if(N>100*HRTREE_OMP_MAX_THREADS) firstprivate(src, buf, conv, key_bytes) num_threads(numt)
    {
      int prefix[UINT8_MAX+1];
      const int nt = omp_get_num_threads();
      const int tid = omp_get_thread_num();
      const int chunk = N / nt;
      const int i0 = tid * chunk;
      const int i1 = (tid == nt-1) ? N : i0 + chunk;
      const int n = i1 - i0;
      int* const bins = Bins[tid];
      for (int byte = 0; byte < key_bytes; ++byte)
      {
        memset(bins, 0, (UINT8_MAX+1)*sizeof(int));
        auto src0(zip::head(src));
        for (int i = i0; i < i1; ++i)
        {
          std::uint8_t key = *(conv(*(src0 + i)) + byte);
          ++bins[key];
        }
        int sb = -1;
        for (int b = 0; b <= UINT8_MAX; ++b)
        {
          if (n == bins[b]) { sb = b; break; }
        }
        SingularBin[tid] = sb;

#       pragma omp barrier

        // Global skip?
        bool skip = SingularBin[0] >= 0;
        for (int t=1; t<nt; ++t) skip &= (SingularBin[0] == SingularBin[t]);  // all equal 

        if (!skip)
        {
          if (tid == 0) Swaped = !Swaped;   // no flush

          // Reduce global prefix table
          int cumsum = 0;
          for (int b = 0; b <= UINT8_MAX; ++b)
          {
            for (int t=0; t<tid; ++t) cumsum += Bins[t][b];
            prefix[b] = cumsum;
            for (int t=tid; t<nt; ++t) cumsum += Bins[t][b];
          }

          // Scatter
          for (int i = i0; i < i1; ++i)
          {
            std::uint8_t key = *(conv(*(src0 + i)) + byte);
            zip::iter_move(src, buf, i, prefix[key]++);
          }
          std::swap(src, buf);
#         pragma omp barrier 
          ;
        }
      }
    }
    return Swaped;
  }


  template <typename ZIt, typename CONV>
  inline void parallel_inplace_msl_radix_sort_impl(ZIt src, const int N, CONV conv, int byte)
  {
    HRTREE_ALIGN_CACHELINE int bins[UINT8_MAX+1];
    HRTREE_ALIGN_CACHELINE int ends[UINT8_MAX+1];

    byte = inplace_msd_radix_sort_prepare(src, N, conv, byte, bins, ends);
    if (byte > 0)
    {
      const int numt = hrtree_max_num_threads();
#     pragma omp parallel for schedule(dynamic,1) firstprivate(src, conv, byte) num_threads(numt)
      for (int b = 0; b <= UINT8_MAX; ++b)
      {
        int n = ends[b] - bins[b];
        if (32 >= n) insertion_sort_impl(src + bins[b], n, conv_less_cmp<CONV>(conv, byte - 1));
        else inplace_msd_radix_sort_impl(src + bins[b], n, conv, byte - 1);
      }
    }
  }


}  // namespace detail


template < typename ZIt, typename CONV >
inline bool parallel_radix_sort(ZIt first, ZIt last, ZIt buf, CONV conv, int bytes)
{
  int N = int(last - first);
  return (1 < N) ? detail::parallel_radix_sort_impl(first, buf, N, conv, bytes) : false;
}


template < typename ZIt, typename CONV >
inline bool parallel_radix_sort(ZIt first, ZIt last, ZIt buf, CONV conv)
{
  return parallel_radix_sort(first, last, buf, conv, CONV::key_bytes);
}


template < typename ZIt >
inline bool parallel_radix_sort(ZIt first, ZIt last, ZIt buf)
{
  typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
  return parallel_radix_sort(first, last, buf, detail::default_converter<T>());
}


template < typename ZIt, typename CONV >
inline void parallel_inplace_radix_sort(ZIt first, ZIt last, CONV conv)
{  
  int N = int(last - first);
  if (1 < N)
  {
    if (32 >= N) insertion_sort_impl(first, N, detail::conv_less_cmp<CONV>(conv, CONV::key_bytes-1));
    else detail::parallel_inplace_msl_radix_sort_impl(first, N, conv, CONV::key_bytes-1);
  }
}


template < typename ZIt >
inline void parallel_inplace_radix_sort(ZIt first, ZIt last)
{
  typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
  parallel_inplace_radix_sort(first, last, detail::default_converter<T>());
}


}

using sorting::parallel_radix_sort;
using sorting::parallel_inplace_radix_sort;

}

#endif
