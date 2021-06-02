// hrtree/sorting/insertion_sort.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_INSERTION_SORT_HPP
#define HRTREE_SORTING_INSERTION_SORT_HPP

#include <iterator>
#include <functional>
#include <hrtree/config.hpp>
#include <hrtree/zip/zip.hpp>


namespace hrtree { namespace sorting { namespace detail {


  template<typename ZIt, typename Comperator>
  inline void insertion_sort_impl(ZIt first, const size_t N, const Comperator& cmp)
  {
    using namespace ::hrtree::zip;

    for (size_t i = 1; i < N; ++i) 
    { 
      if ( cmp(*head(first + i), *head(first + i - 1)) )
      { 
        typename std::iterator_traits<ZIt>::value_type pivot = *(first + i); 
        *(first + i) = *(first + i-1); 
        size_t j = i-1;
        for (; j > 0 && cmp(head(pivot), *head(first + j-1)); --j) { 
          *(first + j) = *(first + j-1); 
        } 
        *(first + j) = pivot;
      }
    }
  }

}

  //! Sorts the data in [first,last) using the given comparator 
  template<typename ZIt, typename Comperator>
  inline void insertion_sort(ZIt first, ZIt last, const Comperator& cmp)
  {
    detail::insertion_sort_impl(first, last - first, cmp);
  }


  //! Sorts the data in [first,last) using the default comparator 
  template<typename ZIt>
  inline void insertion_sort(ZIt first, ZIt last)
  {
    typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
    detail::insertion_sort_impl(first, last - first, std::less<T>());
  }
  
}

using sorting::insertion_sort;

}

#endif
