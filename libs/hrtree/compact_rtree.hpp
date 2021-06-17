// hrtree/compact_rtree.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_COMPACT_RTREE_HPP
#define HRTREE_COMPACT_RTREE_HPP

#include <hrtree/rtree_base.hpp>
#include <hrtree/mbr_build_policy.hpp>


namespace hrtree {


  template <
    typename BV,
    typename BP = mbr_build_policy<BV>,
    size_t FANOUT = 8,
    typename A = typename aligned_allocator< BV, HRTREE_ALIGNOF(BV) >
  >
  class compact_rtree : public rtree_base<BV, BP, FANOUT, A>
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
    compact_rtree(): elems_(0) {}
    ~compact_rtree() {}

    compact_rtree& operator = (const compact_rtree& rhs) { base_type::operator=(rhs); elems_ = rhs.elems_; return *this; }
    void swap(compact_rtree& rhs) { base_type::swap(static_cast<base_type&>(rhs)); }
    void clear() { base_type::clear(); }

    compact_rtree(compact_rtree&& rhs) : base_type(std::move(rhs)) {}
    compact_rtree& operator = (compact_rtree&& rhs) { base_type::operator=(std::move(rhs)); elems_ = rhs.elems_; return *this; }
    void swap(compact_rtree&& rhs) { base_type::swap(std::move(rhs)); }

    template <typename FwdIt>
    void parallel_build(FwdIt first, FwdIt last)
    {
      parallel_build(first, last, base_type::identity_conversion());
    }

    template <typename FwdIt, typename Conversion>
    void parallel_build(FwdIt first, FwdIt last, Conversion conv);

    template <typename FwdIt>
    void build(FwdIt first, FwdIt last)
    {
      build(first, last, base_type::identity_conversion());
    }

    template <typename FwdIt, typename Conversion>
    void build(FwdIt first, FwdIt last, Conversion conv);

    template <typename FwdIt, typename CullPolicy, typename QueryFun>
    void query(FwdIt cfirst, const CullPolicy& cull_policy, QueryFun& query_fun) const;

    template <typename CullPolicy, typename QueryFun>
    void query(const CullPolicy& cull_policy, QueryFun& query_fun) const;

  private:
    size_t elems_;
  };


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename Conversion>
  void compact_rtree<BV,BP,FANOUT,A>::parallel_build(FwdIt first, FwdIt last, Conversion conv)
  {
    elems_ = std::distance(first, last);
    base_type::build_index((elems_ - 1 + FANOUT) / FANOUT);

    // Construct leaf bounding volumes
    const int L = (int)this->leaf_nodes();
    const int N = (int)elems_;
    build_policy bp;
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel for num_threads(numt)
    for (int i=0; i<L; ++i)
    {
      auto dst = this->index_[0] + i;
      this->alloc_.construct(&*dst, conv(*(first + i * FANOUT)));
      const size_t F = std::min<size_t>(FANOUT, N - i * FANOUT);
      for (size_t j=1; j<F; ++j)
      {
        bv_type bv(conv(*(first + i * FANOUT + j)));
        bp(&bv, &bv + 1, &*dst);
      }
    }

    base_type::parallel_build_hierarchy();
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename Conversion>
  void compact_rtree<BV,BP,FANOUT,A>::build(FwdIt first, FwdIt last, Conversion conv)
  {
    elems_ = std::distance(first, last);
    base_type::build_index((elems_ - 1 + FANOUT) / FANOUT);

    // Construct leaf bounding volumes
    const size_t L = this->leaf_nodes();
    bv_iterator dst(this->index_[0]);      
    build_policy bp;
    for (size_t i=0; i<L; ++i)
    {
      this->alloc_.construct(&*dst, conv(*first++));
      const size_t F = std::min<size_t>(FANOUT, elems_ - i * FANOUT);
      for (size_t j=1; j<F; ++j)
      {
        bv_type bv(conv(*first++));
        bp(&bv, &bv + 1, &*dst);
      }
      ++dst;
    }
    base_type::build_hierarchy();
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  template <typename FwdIt, typename CullPolicy, typename QueryFun>
  void compact_rtree<BV,BP,FANOUT,A>::query(
    FwdIt cfirst,
    const CullPolicy& cull_policy,
    QueryFun& query_fun
    ) const
  {
    if (this->empty()) return;
    size_t level = base_type::height_ - 1;
    stack_element stack[base_type::MaxHeight];
    stack[level] = base_type::stack_element(0,1);
    while (level < base_type::height_)
    {
      base_type::stack_element& s = stack[level];
      const_bv_iterator first( index_[level] + s.first );
      s.second = std::min(s.second, level_nodes(level));
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
          if (level > 0)
          {
            stack[--level] = base_type::stack_element(next_level_first, s.first * FANOUT);
            goto descent;
          }
          size_t i0 = next_level_first;
          const size_t i1 = std::min(s.first * FANOUT, elems_); 
          FwdIt it(cfirst);
          std::advance(it, i0);
          for (; i0 < i1; ++i0, ++it)
          {
            query_fun(*it);
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
  void compact_rtree<BV, BP, FANOUT, A>::query(
    const CullPolicy& cull_policy,
    QueryFun& query_fun
    ) const
  {
    if (this->empty()) return;
    size_t level = base_type::height_ - 1;
    stack_element stack[base_type::MaxHeight];
    stack[level] = base_type::stack_element(0, 1);
    while (level < base_type::height_)
    {
      base_type::stack_element& s = stack[level];
      const_bv_iterator first(this->index_[level] + s.first);
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
          if (level > 0)
          {
            stack[--level] = base_type::stack_element(next_level_first, s.first * FANOUT);
            goto descent;
          }
          size_t i0 = next_level_first;
          const size_t i1 = std::min(s.first * FANOUT, elems_);
          for (; i0 < i1; ++i0)
          {
            query_fun(i0);
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
