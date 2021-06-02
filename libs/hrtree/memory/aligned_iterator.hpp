// hrtree/memory/aligned_iterator.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_MEMORY_ALIGNED_ITERATOR_INCLUDED
#define HRTREE_MEMORY_ALIGNED_ITERATOR_INCLUDED

#include <iterator>
#include <type_traits>
#include <hrtree/config.hpp>


namespace hrtree { namespace memory {


  template <
    typename T,
    size_t ALIGN = HRTREE_ALIGNOF(T)
  >
  class aligned_iterator
  {
  public:
    static const size_t align = ALIGN;
    static const size_t aligned_size = (sizeof(T) + (align - 1)) & ~(align - 1);

  private:
    struct aligned_storage { char buf[aligned_size]; };

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    aligned_iterator(const aligned_iterator& x) : base_(x.base_) {}
    explicit aligned_iterator(T* base = 0) : base_((aligned_storage*)(void*)(base)) {}

    aligned_iterator& operator=(const aligned_iterator& x) { base_ =  x.base_;  return *this; }
    aligned_iterator& operator=(T* base) { base_ = (aligned_storage*)(void*)(base); return *this; }
    
    reference operator * () const { return *(T*)(void*)(base_); }
    pointer operator -> () const { return (T*)(void*)(base_); }
    reference operator [] (size_t i) const { return *(T*)(void*)(base_ + i); }

    aligned_iterator& operator ++ () { ++base_; return *this; }
    aligned_iterator& operator -- () { --base_; return *this; }
    aligned_iterator operator ++ (int) { auto tmp(*this); ++base_; return tmp; }
    aligned_iterator operator -- (int) { auto tmp(*this); --base_; return tmp; }
    aligned_iterator& operator += (ptrdiff_t n) { base_ += n; return *this; }
    aligned_iterator& operator -= (ptrdiff_t n) { base_ -= n; return *this; }

    aligned_iterator operator + (ptrdiff_t n) const { auto tmp(*this); return tmp += n; }
    aligned_iterator operator - (ptrdiff_t n) const { auto tmp(*this); return tmp -= n; }

    bool operator == (const aligned_iterator& x) const { return base_ == x.base_; }
    bool operator != (const aligned_iterator& x) const { return base_ != x.base_; }
    bool operator < (const aligned_iterator& x) const { return base_ < x.base_; }
    bool operator >= (const aligned_iterator& x) const { return ! (base_ < x.base_); }

    friend ptrdiff_t operator - (const aligned_iterator& x, const aligned_iterator& y)
    {
      return x.base_ - y.base_;
    }

    void swap(aligned_iterator& rhs)
    {
      std::swap(base_, rhs.base_);
    }

  private:
    // Grant access to const interator
    friend class aligned_iterator< const T, align >;

    aligned_storage* base_;
  };


  template <
    typename T,
    size_t ALIGN
  >
  class aligned_iterator< const T, ALIGN >
  {
  public:
    static const size_t align = ALIGN;
    static const size_t aligned_size = (sizeof(T) + (align - 1)) & ~(align - 1);

  private:
    struct aligned_storage { char buf[aligned_size]; };

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = const T;
    using difference_type = ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    aligned_iterator(const aligned_iterator& x) : base_(x.base_) {}
    aligned_iterator(const aligned_iterator< T, align >& x) : base_((aligned_storage*)(void*)(x.base_)) {}
    explicit aligned_iterator(const T* base = 0) : base_((aligned_storage*)(void*)(base)) {}

    aligned_iterator& operator=(const aligned_iterator& x) { base_ = x.base_;  return *this; }
    aligned_iterator& operator=(const aligned_iterator< T, align >& x) { base_ = (aligned_storage*)(void*)(x.base_); return *this; }
    aligned_iterator& operator=(const T* base) { base_ = (aligned_storage*)(void*)(base); return *this; }

    const T& operator * () const { return *(T*)(void*)(base_); }
    const T* operator -> () const { return (T*)(void*)(base_); }

    aligned_iterator& operator ++ () { ++base_; return *this; }
    aligned_iterator& operator -- () { --base_; return *this; }
    aligned_iterator operator ++ (int) { auto tmp(*this); ++base_; return tmp; }
    aligned_iterator operator -- (int) { auto tmp(*this); --base_; return tmp; }
    aligned_iterator& operator += (ptrdiff_t n) { base_ += n; return *this; }
    aligned_iterator& operator -= (ptrdiff_t n) { base_ -= n; return *this; }

    aligned_iterator operator + (ptrdiff_t n) const { auto tmp(*this); return tmp += n; }
    aligned_iterator operator - (ptrdiff_t n) const { auto tmp(*this); return tmp -= n; }

    bool operator == (const aligned_iterator& x) const { return base_ == x.base_; }
    bool operator != (const aligned_iterator& x) const { return base_ != x.base_; }
    bool operator < (const aligned_iterator& x) const { return base_ < x.base_; }
    bool operator >= (const aligned_iterator& x) const { return !(base_ < x.base_); }

    friend ptrdiff_t operator - (const aligned_iterator& x, const aligned_iterator& y)
    {
      return x.base_ - y.base_;
    }

    void swap(aligned_iterator& rhs)
    {
      std::swap(base_, rhs.base_);
    }

  private:
    aligned_storage* base_;
  };


}

using memory::aligned_iterator;

}

#endif
