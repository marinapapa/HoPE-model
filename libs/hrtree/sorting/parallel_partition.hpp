// hrtree/sorting/parallel_partition.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_PARALLEL_PARTITION_HPP
#define HRTREE_SORTING_PARALLEL_PARTITION_HPP

#include <hrtree/config.hpp>
#include <hrtree/sorting/partition.hpp>


namespace hrtree { namespace detail { 


  template <int Block>
  inline int& incr(int& i, int stride)
  {
    ++i; if (0 == (i % Block)) i += stride - Block;
    return i; 
  }

  
  template <int Block>
  inline int& decr(int& i, int stride)
  {
    if (0 == (i % stride)) i -= stride - Block;
    return --i; 
  }
  

  template <int Block, typename ZIt, typename Predicate>
  inline int block_partition(ZIt first, ZIt last, int stride, Predicate pred)
  {
    using namespace hrtree::zip;

    const int N = int(last - first);
    int i0 = 0;
    int i1 = N;
    int strides = i1 / stride;
    int lastBlock = stride * strides + Block;
    i1 = std::min<int>(lastBlock, i1);

    for (;; incr<Block>(i0, stride))
    {
      for (; i0 < i1 && pred(*head(first + i0)); incr<Block>(i0, stride))
        ;
      if (i0 >= i1)
        break;
      for (; (i0 < decr<Block>(i1, stride)) && !pred(*head(first + i1)); )
        ;
      if (i0 >= i1)
        break;
      std::iter_swap(first + i0, first + i1);
    }
    return std::min<int>(i0, N);
  }


}  // namespace detail


  //! Classifies elements in a range into two disjoint sets, with those elements satisfying
  //! a unary predicate preceding those that fail to satisfy it.
  template<typename ZIt, typename Predicate>
  inline ZIt parallel_partition(ZIt first, ZIt last, Predicate pred)
  {
    int mt = std::min<int>(omp_get_max_threads(), HRTREE_OMP_MAX_THREADS);
    const int mstride = mt * HRTREE_PARALLEL_PARTITION_BLOCK;
    if (mt < 2 || ((last - first) < mstride))
    {
      return ::hrtree::partition(first, last, pred);
    }
    int pivot[HRTREE_OMP_MAX_THREADS];
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel firstprivate(first, last) num_threads(numt)
    {
      const int nt = omp_get_num_threads();
      const int tid = omp_get_thread_num();
      const int stride = nt * HRTREE_PARALLEL_PARTITION_BLOCK;
      const int ofs = tid * HRTREE_PARALLEL_PARTITION_BLOCK;
      pivot[tid] = ofs + detail::block_partition<HRTREE_PARALLEL_PARTITION_BLOCK>(
        first + ofs, last, stride, pred
        );
      if (tid == 0) mt = nt;
    }
    // Cleanup
    std::pair<const int*, const int*> c = std::minmax_element(pivot, pivot + mt);
    return ::hrtree::partition(first + *c.first, first + *c.second, pred);
  }


}


#endif
