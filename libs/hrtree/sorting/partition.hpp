// hrtree/sorting/partition.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_SORTING_PARTITION_HPP
#define HRTREE_SORTING_PARTITION_HPP

#include <iterator>
#include <algorithm>
#include <hrtree/config.hpp>
#include <hrtree/zip/zip.hpp>


namespace hrtree { namespace sorting { namespace detail {
  
  template<typename RaIt, typename Cmp>
  inline size_t median_of_three(Cmp cmp, RaIt a, size_t l, size_t m, size_t r)
  {
    return cmp(a[l], a[m]) ? ( cmp(a[m], a[r]) ? m : ( cmp(a[l], a[r]) ? r : l ) ) 
      : ( cmp(a[r], a[m]) ? m : ( cmp(a[r], a[l]) ? r : l ) );
  }
  
  template<typename RaIt, typename Cmp>
  inline size_t pseudo_median_of_nine(RaIt first, RaIt last, Cmp cmp)
  {
    const size_t size = std::distance(first, last);
    size_t offset = size/8u;
    return median_of_three(cmp, first, 
      median_of_three(cmp, first, 0, offset, offset*2),
      median_of_three(cmp, first, offset*3, offset*4, offset*5),
      median_of_three(cmp, first, offset*6, offset*7, size - 1));
  }

}


  template<typename ZIt, typename Cmp>
  inline ZIt unguarded_partition(ZIt first, ZIt last, Cmp cmp)
  {
    using namespace ::hrtree::zip;

    const size_t m = detail::pseudo_median_of_nine(head(first), head(last), cmp);
    typename std::iterator_traits< 
      typename head_type<ZIt>::type 
    >::value_type pivot = *(head(first) + m);
    
    for (; ; ++first)
    {
      for (; first != last && cmp(*head(first), pivot); ++first)
        ;
      if (first == last)
        break;  // done
      for (; first != --last && cmp(pivot, *head(last)); )
        ;
      if (first == last)
        break;  // done

      std::iter_swap(first, last);
    }
    return (first);
  }


  template<typename ZIt>
  inline ZIt unguarded_partition(ZIt first, ZIt last)
  {
    typedef typename std::iterator_traits< typename zip::head_type<ZIt>::type >::value_type T;
    return unguarded_partition(first, last, std::less<T>());
  }


  template<typename ZIt, typename Predicate>
  inline ZIt partition(ZIt first, ZIt last, Predicate pred)
  {
    using namespace ::hrtree::zip;

    for (; ; ++first)
    {
      for (; first != last && pred(*head(first)); ++first)
        ;
      if (first == last)
        break;  // done
      for (; first != --last && !pred(*head(last)); )
        ;
      if (first == last)
        break;  // done

      std::iter_swap(first, last);
    }
    return (first);
  }

}

using sorting::partition;
using sorting::unguarded_partition;

}

#endif
