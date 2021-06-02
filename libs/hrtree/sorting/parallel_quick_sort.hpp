// hrtree/sorting/parallel_quick_sort.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_PARALLEL_QUICK_SORT_HPP
#define HRTREE_SORTING_PARALLEL_QUICK_SORT_HPP

#include <iterator>
#include <algorithm>
#include <stack>
#include <hrtree/config.hpp>
#include <hrtree/zip/zip.hpp>
#include <hrtree/sorting/quick_sort.hpp>
#include <hrtree/sorting/parallel_partition.hpp>
#include <hrtree/arch/select.hpp>                   // _mm_pause


namespace hrtree { namespace sorting { namespace detail {


  template <typename T, typename Cmp>
  struct bindCmp
  {
    bindCmp(T pivot, const Cmp& cmp): pivot_(pivot), cmp_(cmp) {}
    bool operator()(const T& x) const { return cmp_(x, pivot_); }
    T pivot_;
    const Cmp& cmp_;
  };

}  // namespace detail


  //! Sorts the data in [first,last) using the given comparator 
  template<typename ZIt, typename Cmp>
  inline void parallel_quick_sort(ZIt first, ZIt last, Cmp cmp)
  {
    using namespace ::hrtree::zip;

    const size_t size = last - first;
    if (32 >= size) return detail::insertion_sort_impl(first, size, cmp);

    std::stack< std::pair<ZIt, ZIt> > stack;
    const size_t m = detail::pseudo_median_of_nine(head(first), head(last), cmp);
    detail::bindCmp< typename std::iterator_traits< 
      typename head_type<ZIt>::type 
    >::value_type, Cmp > pred(*(head(first) + m), cmp);
    
    ZIt mid = parallel_partition(first, last, pred);
    stack.push(std::make_pair(first, mid));
    stack.push(std::make_pair(mid, last));
    int Load = 0;
    bool Empty = 0;
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel num_threads(numy)
    {
      std::pair< ZIt, ZIt > part;
      bool empty = false;
      while (Load || !Empty)
      {
#       pragma omp critical (hrtree_parallel_quick_sort)
        {
          empty = Empty = stack.empty();
          if (!empty) 
          {
            part = stack.top();
            stack.pop();
#           pragma omp atomic
            ++Load;
          } 
        }
        if (!empty) 
        {
          {
            const size_t size = part.second - part.first;
            if (size < 1000) 
            {
              hrtree::quick_sort(part.first, part.second, cmp);
            }
            else
            {
              ZIt mid = hrtree::unguarded_partition(part.first, part.second, cmp);
#             pragma omp critical (hrtree_parallel_quick_sort)
              {
                stack.push(std::make_pair(part.first, mid));
                stack.push(std::make_pair(mid, part.second));
                Empty = false;
              }
            }
          }
#         pragma omp atomic
          --Load;
        }
#       pragma omp flush(Load)
        // Spin wait for more work
        while (Load && Empty)
        {
#         pragma omp flush(Load, Empty)
          _mm_pause();
        }
      }
    }
  }


  //! Sorts the data in [first,last) using the default comparator 
  template<typename ZIt>
  inline void parallel_quick_sort(ZIt first, ZIt last)
  {
    typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
    parallel_quick_sort(first, last, std::less<T>());
  }


}

using sorting::parallel_quick_sort;

}

#endif
