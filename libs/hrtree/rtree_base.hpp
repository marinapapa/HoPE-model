// hrtree/rtree_base.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_RTREE_BASE_HPP
#define HRTREE_RTREE_BASE_HPP

#include <exception>
#include <mutex>
#include <cassert>
#include <iterator>
#include <utility>
#include <algorithm>
#include <hrtree/config.hpp>
#include <hrtree/memory/aligned_memory.hpp>
#include <hrtree/memory/aligned_iterator.hpp>


namespace hrtree { 


  template <typename BV,
        typename BP,
        size_t FANOUT,
        typename A
  >
  class rtree_base
  {
  protected:
    typedef aligned_iterator< BV, memory::allocator_has_align< A >::value > aligned_iter;
    typedef aligned_iterator< const BV, memory::allocator_has_align< A >::value > const_aligned_iter;

  public:
    static const size_t MaxHeight = 16;
    static const size_t bv_align = memory::allocator_has_align<A>::value;

    typedef typename aligned_iter::value_type bv_type;
    typedef typename aligned_iter::reference  bv_reference;
    typedef typename aligned_iter::pointer    bv_pointer;
    typedef aligned_iter                      bv_iterator;
    typedef const_aligned_iter                const_bv_iterator;
    typedef BP                                build_policy;
    typedef A                                 allocator_type;

  protected:
    rtree_base() : index_(), height_(0), capacity_(0) {}
    rtree_base(const rtree_base& x);
    rtree_base& operator=(const rtree_base& rhs);
    virtual ~rtree_base();
    void swap(rtree_base& other);
    void clear();

    rtree_base(rtree_base&& x);
    void swap(rtree_base&& other);
    rtree_base& operator=(rtree_base&& rhs);

  public:
    bool empty() const { return 0 == height_; }
    size_t max_nodes() const { return alloc_.max_size(); }
    size_t nodes() const { return (height_) ? index_[height_-1] - index_[0] : 0; }
    size_t buckets() const { return nodes() / FANOUT; }
    size_t leaf_nodes() const { return index_[1] - index_[0]; }
    size_t level_nodes(size_t i) const { assert( i < height_); return index_[i + 1] - index_[i]; }
    size_t max_height() const { return size_t(MaxHeight); }
    size_t height() const { return height_; }
    size_t fanout() const { return size_t(FANOUT); }

    const bv_reference total_bv() const { assert( 0 != height_ ); return *index_[height_ - 1]; }
    const bv_reference leaf_bv(size_t i) const { assert( i < leaf_nodes()); return *(index_[0] + i); }
    bv_reference leaf_bv(size_t i) { assert( i < leaf_nodes()); return *(index_[0] + i); }

    const_bv_iterator level_begin(size_t level) const { return index_[level]; }
    const_bv_iterator level_end(size_t level) const { return index_[level + 1]; }

    bv_iterator level_begin(size_t level) { return index_[level]; }
    bv_iterator level_end(size_t level) { return index_[level + 1]; }

    void build_index(size_t n);
    void build_hierarchy();
    void parallel_build_hierarchy();

  protected:
    typedef std::pair< size_t, size_t > stack_element;

    struct identity_conversion
    {
      template <typename T>
      const T& operator()(const T& x) const { return x; }
    };

    bv_iterator  index_[MaxHeight + 1];
    size_t    height_, capacity_;
    allocator_type  alloc_;
  };


  template <typename BV, typename BP, size_t FANOUT, typename A>
  rtree_base<BV,BP,FANOUT,A>::rtree_base(const rtree_base& x)
    : index_(), height_(0), capacity_(0) 
  {
    build_index(x.leaf_nodes());
    bv_iterator dst = index_[0];
    const_bv_iterator src = x.index_[0];
    const int N = (int)nodes();
    for (int i=0; i <= N; ++i)
    {
      alloc_.construct(&*(dst + i), *(src + i));
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  rtree_base<BV, BP, FANOUT, A>& rtree_base<BV, BP, FANOUT, A>::operator=(const rtree_base<BV, BP, FANOUT, A>& rhs)
  {
    rtree_base tmp(rhs);
    swap(tmp);
    return *this;
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  rtree_base<BV,BP,FANOUT,A>::~rtree_base()
  { 
    if (0 != capacity_)
    {
      memory::aligned_destruct<BV>(index_[0], nodes());  // orphan remaining nodes
      alloc_.deallocate(&*index_[0], capacity_);
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  void rtree_base<BV,BP,FANOUT,A>::swap(rtree_base<BV, BP,FANOUT,A>& other)
  {
    if (this != &other)
    {
      for (size_t i=0; i<max_height(); ++i)
      {
        std::swap(index_[i], other.index_[i]);
      }
      std::swap(height_, other.height_);
      std::swap(capacity_, other.capacity_);
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  rtree_base<BV,BP,FANOUT,A>::rtree_base(rtree_base&& rhs)
    : index_(), height_(0), capacity_(0) 
  {
    swap(std::forward<rtree_base>(rhs));
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  rtree_base<BV, BP, FANOUT, A>& rtree_base<BV, BP, FANOUT, A>::operator=(rtree_base<BV, BP, FANOUT, A>&& rhs)
  {
    if (this != &rhs)
    {
      clear();
      swap(std::forward<rtree_base>(rhs));
    }
    return *this;
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  void rtree_base<BV,BP,FANOUT,A>::swap(rtree_base<BV, BP,FANOUT,A>&& other)
  {
    if (this != &other)
    {
      for (size_t i=0; i<max_height(); ++i)
      {
        index_[i] = other.index_[i]; other.index_[i] = 0;
      }
      height_ = other.height_; other.height_ = 0;
      capacity_ = other.capacity_; other.capacity_ = 0;
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  void rtree_base<BV,BP,FANOUT,A>::clear()
  {
    rtree_base tmp;
    swap(tmp);
  }
  
  
  template <typename BV, typename BP, size_t FANOUT, typename A>
  void rtree_base<BV,BP,FANOUT,A>::build_index(size_t n)
  {
    memory::aligned_destruct<BV>(index_[0], nodes());
    if (0 == n)
    {
      height_ = 0;
      index_[1] = index_[0];
      return;
    }
    if (n != leaf_nodes()) 
    {
      size_t offs[MaxHeight+1] = {};
      size_t N = 0;
      size_t level = 1;
      do
      {
        offs[level++] = (N += n);
        n = (n-1+FANOUT)/FANOUT;
      } while (n > 1);
      offs[level] = ++N;
      if (N > capacity_)
      {
        alloc_.deallocate(&*index_[0], capacity_);
        height_ = capacity_ = 0;
        index_[0] = bv_iterator(alloc_.allocate(N));
        capacity_ = N;
      }

      // Initialize the index array.
      // [index_[i], index_[i+1]) <- [begin, end) - range of bounding volumes in level i.
      //
      height_ = level;
      for (level = 1; level <= height_; ++level)
      {
        index_[level] = index_[0] + offs[level];
      }
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  void rtree_base<BV,BP,FANOUT,A>::parallel_build_hierarchy()
  {
    std::mutex emutex;
    std::exception_ptr eptr;

    for (size_t level = 1; level < height_; ++level)
    {
      bv_iterator dst = index_[level];
      const_bv_iterator first = index_[level-1];
      const_bv_iterator last = dst;
      const int N = (int)level_nodes(level);
#     pragma omp parallel firstprivate(dst)
      {
        try
        {
          build_policy buildPolicy;
#         pragma omp for schedule(static) nowait
          for (int i = 0; i < N; ++i)
          {
            const_bv_iterator src = first + i * FANOUT;
            alloc_.construct(&*(dst + i), *src);
            buildPolicy(src + 1, std::min(src + FANOUT, last), dst + i);
          }
        }
        catch (...)
        {
          std::lock_guard<std::mutex> lock(emutex);
          eptr = std::current_exception();
        }
      }
      if (eptr != nullptr) std::rethrow_exception(eptr);
    }
  }


  template <typename BV, typename BP, size_t FANOUT, typename A>
  void rtree_base<BV,BP,FANOUT,A>::build_hierarchy()
  {
    build_policy buildPolicy;
    for (size_t level = 1; level < height_; ++level)
    {
      bv_iterator dst = index_[level];
      const_bv_iterator first = index_[level-1];
      const_bv_iterator last = dst;
      const size_t n = level_nodes(level);
      for (size_t i = 0; i < n; ++i)
      {
        const_bv_iterator src = first + i * FANOUT;
        alloc_.construct(&*(dst + i), *src);
        buildPolicy(src + 1, std::min(src + FANOUT, last), dst + i);
      }
    }
  }


}


#endif
