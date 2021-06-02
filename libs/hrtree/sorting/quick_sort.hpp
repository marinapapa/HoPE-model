// hrtree/sorting/quick_sort.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_QUICK_SORT_HPP
#define HRTREE_SORTING_QUICK_SORT_HPP

#include <iterator>
#include <algorithm>
#include <stack>
#include <hrtree/config.hpp>
#include <hrtree/sorting/insertion_sort.hpp>
#include <hrtree/sorting/partition.hpp>


namespace hrtree { namespace sorting {


  //! Sorts the data in [first,last) using the given comparator 
  template <typename ZIt, typename Cmp >
  inline void quick_sort(ZIt left, ZIt right, Cmp cmp)
  {
    using namespace hrtree::zip;

    const size_t size = right - left;
    if (size > 1)
    {
      if (size < 32) 
      {
        detail::insertion_sort_impl(left, size, cmp);
      }
      else
      {
        ZIt mid = unguarded_partition(left, right, cmp);
        if (left != mid) quick_sort(left, mid, cmp);
        if (mid != right) quick_sort(mid, right, cmp);
      }
    }
  }


  //! Sorts the data in [first,last) using the default comparator 
  template<typename ZIt>
  inline void quick_sort(ZIt first, ZIt last)
  {
    typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
    quick_sort(first, last, std::less<T>());
  }


}

using sorting::quick_sort;

}

#endif
