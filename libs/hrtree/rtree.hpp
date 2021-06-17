// hrtree/rtree.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_RTREE_HPP
#define HRTREE_RTREE_HPP

#include <hrtree/rtree_base.hpp>
#include <hrtree/mbr_build_policy.hpp>


namespace hrtree {


  template <
    typename BV,
    typename BP = mbr_build_policy<BV>,
    size_t FANOUT = 8,
    typename A = aligned_allocator< BV, HRTREE_ALIGNOF(BV) >
  >
  class rtree : public rtree_base<BV, BP, FANOUT, A>
  {
    typedef rtree_base<BV,BP,FANOUT,A> base_type;
    using base_type::stack_element;
  
  public:
    using base_type::bv_align;
    using base_type::bv_type;
    using base_type::bv_reference;
    using base_type::bv_iterator;
    using base_type::const_bv_iterator;
    using base_type::build_policy;
  
  public:
    rtree()  {}
    ~rtree() {}
    rtree(const rtree& x) : base_type(x) {}

    rtree& operator = (const rtree& rhs) { base_type::operator=(rhs); return *this; }
    void swap(rtree& rhs) { base_type::swap(rhs); }
    void clear() { base_type::clear(); }

    rtree(rtree&& rhs) : base_type(std::move(rhs)) {}
    rtree& operator = (rtree&& rhs) { base_type:operator=(std::move(rhs)); return *this; }
    void swap(rtree&& other) { base_type::swap(std::forward(other)); }

    template <typename FwIt, typename Conversion>
    void build(FwIt first, FwIt last, Conversion conv);

    template <typename FwdIt>
    void build(FwdIt first, FwdIt last)
    {
      build(first, last, base_type::identity_conversion());
    }

    template <typename FwdIt>
    void parallel_build(FwdIt first, FwdIt last)
    {
      parallel_build(first, last, base_type::identity_conversion());
    }

    template <typename FwdIt, typename Conversion>
    void parallel_build(FwdIt first, FwdIt last, Conversion conv);

    template <typename FwdIt, typename Constructor>
    void construct(FwdIt first, FwdIt last, Constructor ctor);

    template <typename FwdIt, typename Constructor>
    void parallel_construct(FwdIt first, FwdIt last, Constructor ctor);

    template <typename FwdIt, typename CullPolicy, typename QueryFun>
    void query(FwdIt cfirst, const CullPolicy& cull_policy, QueryFun& query_fun) const;

    template <typename CullPolicy, typename QueryFun>
    void query(const CullPolicy& cull_policy, QueryFun& query_fun) const;
  };


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename Conversion>
  void rtree<BV,BP,FANOUT,A>::parallel_build(FwdIt first, FwdIt last, Conversion conv)
  {
    std::mutex emutex;
    std::exception_ptr eptr;

    base_type::build_index(std::distance(first, last));

    // Construct leaf bounding volumes
    const int N = static_cast<int>(this->leaf_nodes());
#   pragma omp parallel firstprivate(first, conv)
    {
      try
      {
#       pragma omp for schedule(static)
        for (int i=0; i<N; ++i)
        {
          FwdIt src(first);
          std::advance(src, i);
          this->alloc_.construct(&*(this->index_[0] + i), conv(*src));
        }
      }
      catch (...)
      {
        std::lock_guard<std::mutex> lock(emutex);
        eptr = std::current_exception();
      }
    }
    if (eptr != nullptr) std::rethrow_exception(eptr);
    base_type::parallel_build_hierarchy();
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename Conversion>
  void rtree<BV,BP,FANOUT,A>::build(FwdIt first, FwdIt last, Conversion conv)
  {
    base_type::build_index(std::distance(first, last));

    // Construct leaf bounding volumes
    const int N = (int)this->leaf_nodes();
    typename base_type::bv_iterator dst(this->index_[0]);
    for (int i=0; i<N; ++i)
    {
      this->alloc_.construct(&*dst, conv(*first));
      ++dst;
      ++first;
    }
    base_type::build_hierarchy();
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename Constructor>
  void rtree<BV,BP,FANOUT,A>::construct(FwdIt first, FwdIt last, Constructor ctor)
  {
    typename base_type::build_index(std::distance(first, last));

    // Construct leaf bounding volumes
    const int N = (int)this->leaf_nodes();
    typename base_type::bv_iterator dst(this->index_[0]);
    for (int i=0; i<N; ++i)
    {
      ctor(&*dst, *first);
      ++dst;
      ++first;
    }
    base_type::build_hierarchy();
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename Constructor>
  void rtree<BV,BP,FANOUT,A>::parallel_construct(FwdIt first, FwdIt last, Constructor ctor)
  {
    std::mutex emutex;
    std::exception_ptr eptr;

    base_type::build_index(std::distance(first, last));

    // Construct leaf bounding volumes
    const int N = (int)this->leaf_nodes();
#   pragma omp parallel firstprivate(first, ctor)
    {
      try
      {
#       pragma omp for schedule(static)
        for (int i=0; i<N; ++i)
        {
          FwdIt src(first);
          std::advance(src, i);
          ctor(&*(this->index_[0] + i), *src);
        }
      }
      catch (...)
      {
        std::lock_guard<std::mutex> lock(emutex);
        eptr = std::current_exception();
      }
    }
    if (eptr != nullptr) std::rethrow_exception(eptr);
    base_type::parallel_build_hierarchy();
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename CullPolicy, typename QueryFun>
  void rtree<BV,BP,FANOUT,A>::query(
    FwdIt cfirst,
    const CullPolicy& cull_policy,
    QueryFun& query_fun
    ) const
  {
    if (this->empty()) return;
    size_t level = base_type::height_ - 1;
    typename base_type::stack_element stack[base_type::MaxHeight];
    stack[level] = base_type::stack_element(0,1);
    while (level < base_type::height_)
    {
      typename base_type::stack_element& s = stack[level];
      typename base_type::const_bv_iterator first( this->index_[level] + s.first );
      s.second = std::min(s.second, this->level_nodes(level));
      for (; s.first < s.second; ++s.first)
      {
        if (cull_policy(*first++))
        {
          size_t next_level_first = s.first * FANOUT;
          for (++s.first; s.first < s.second; ++s.first)
          {
            if (!cull_policy(*first++))
            {
              break;
            }
          }
          if (level > 1)
          {
            stack[--level] = base_type::stack_element(next_level_first, s.first * FANOUT);
            goto descent;
          }
          FwdIt it(cfirst);
          std::advance(it, next_level_first);
          typename base_type::const_bv_iterator first_leaf(this->index_[0] + next_level_first);
          typename base_type::const_bv_iterator last_leaf = std::min(this->index_[0] + s.first * FANOUT, this->index_[1]);
          for (; first_leaf != last_leaf; ++first_leaf, ++it)
          {
            if (cull_policy(*first_leaf))
            {
              query_fun(*it);
            }
          }
        }
      }
      ++level;
descent:
      ;
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename CullPolicy, typename QueryFun>
  void rtree<BV, BP, FANOUT, A>::query(
    const CullPolicy& cull_policy,
    QueryFun& query_fun
    ) const
  {
    if (this->empty()) return;
    size_t level = base_type::height_ - 1;
    typename base_type::stack_element stack[base_type::MaxHeight];
    stack[level] = typename base_type::stack_element(0, 1);
    while (level < base_type::height_)
    {
      typename base_type::stack_element& s = stack[level];
      typename base_type::const_bv_iterator first(this->index_[level] + s.first);
      s.second = std::min(s.second, this->level_nodes(level));
      for (; s.first < s.second; ++s.first)
      {
        if (cull_policy(*first++))
        {
          size_t next_level_first = s.first * FANOUT;
          for (++s.first; s.first < s.second; ++s.first)
          {
            if (!cull_policy(*first++))
            {
              break;
            }
          }
          if (level > 1)
          {
            stack[--level] = typename base_type::stack_element(next_level_first, s.first * FANOUT);
            goto descent;
          }
          size_t leaf_idx = next_level_first;
          typename base_type::const_bv_iterator first_leaf(this->index_[0] + next_level_first);
          typename base_type::const_bv_iterator last_leaf = std::min(this->index_[0] + s.first * FANOUT, this->index_[1]);
          for (; first_leaf != last_leaf; ++first_leaf, ++leaf_idx)
          {
            if (cull_policy(*first_leaf))
            {
              query_fun(leaf_idx);
            }
          }
        }
      }
      ++level;
    descent:
      ;
    }
  }


}  // namespace hrtree


#endif
