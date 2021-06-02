// hrtree/memory/aligned_buffer.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

#ifndef HRTREE_MEMORY_ALIGNED_BUFFER_HPP
#define HRTREE_MEMORY_ALIGNED_BUFFER_HPP

#include <cassert>
#include <type_traits>
#include <hrtree/config.hpp>
#include <hrtree/memory/aligned_memory.hpp>
#include <hrtree/memory/aligned_iterator.hpp>


namespace hrtree { namespace memory {


  template <
    typename T,
    size_t ALIGN = HRTREE_ALIGNOF(T),
    typename ALLOCATOR = aligned_allocator< typename std::remove_const<T>::type, ALIGN >
  >
  class aligned_buffer
  {
  public:
    typedef ALLOCATOR allocator_type;     //!< The allocator type

    typedef aligned_iterator<typename std::remove_const<T>::type, ALIGN > iterator;        //!< The random access iterator
    typedef aligned_iterator<typename std::add_const<T>::type, ALIGN >    const_iterator;  //!< The random access const iterator

    typedef typename allocator_type::value_type       value_type;       //!< The type of object, T, stored in the buffer
    typedef typename allocator_type::pointer          pointer;          //!< Pointer to T
    typedef typename allocator_type::const_pointer    const_pointer;    //!< Const pointer to T
    typedef typename allocator_type::reference        reference;        //!< Reference to T
    typedef typename allocator_type::const_reference  const_reference;  //!< Const reference to T
    typedef typename allocator_type::difference_type  difference_type;  //!< An unsigned integral type
    typedef typename allocator_type::size_type        size_type;        //!< An signed integral type


    //! Constructs buffer.
    aligned_buffer() : buf_(0), size_(0), capacity_(0), alloc_()
    {
    }
    

    //! Constructs buffer with N default constructed values.
    explicit aligned_buffer(const size_t N) : buf_(0), size_(0), capacity_(0), alloc_()
    {
      resize(N);
    }
    

    //! Copy constructor.
    aligned_buffer(const aligned_buffer& x) : buf_(0), size_(0), capacity_(0), alloc_()
    {
      resize(x.size());
      aligned_construct_iter<value_type>(buf_, size_, x.begin());
    }


    //! Assignment.
    aligned_buffer& operator=(const aligned_buffer& x)
    {
      if (this != &x)
      {
        aligned_buffer tmp(x);
        swap(tmp);
      }
      return *this;
    }


    //! Move constructor.
    aligned_buffer(aligned_buffer&& x) 
      : buf_(x.buf_), size_(x.size_), capacity_(x.capacity_), alloc_()
    {
      x.buf_ = 0; x.size_ = x.capacity_ = 0;
    }


    //! Move assignment.
    aligned_buffer& operator=(aligned_buffer&& rhs)
    {
      if (this != &rhs)
      {
        orphan();
        buf_ = rhs.buf_; size_ = rhs.size_; capacity_ = rhs.capacity_;
        rhs.buf_ = 0; rhs.size_ = rhs.capacity_ = 0;
      }
      return *this;
    }


    //! Destructor.
    ~aligned_buffer()
    {
      orphan();
    }


    //! Destroys all objects in the buffer.
    void clear()
    {
      aligned_destruct<value_type>(buf_, size_);
      size_ = 0;
    }


    //! Shrink to hold exactly size() objects.
    void shrink_to_fit()
    {
      if (size_ != capacity_)
      {
        aligned_buffer tmp(*this);
        swap(tmp);
      }
    }


    //! Reserves memory for N objects.
    void reserve(size_t N)
    {
      if (N > capacity_)
      {
        iterator tmp = iterator((pointer)alloc_.allocate(N));
        aligned_construct_iter<value_type>(tmp, size_, buf_);
        orphan();
        buf_ = tmp;
        capacity_ = N;
      }
    }


    //! Resize to N objects. 
    //! Added objects are default constructed.
    //! Superfluous objects are destructed. 
    void resize(size_t N)
    {
      if (N < size_)
      {
        aligned_destruct<value_type>(buf_ + N, size_ - N);
      }
      if (N > capacity_)
      {
        reserve(N);
        capacity_ = N;
      }
      if (N > size_)
      {
        aligned_default_construct<value_type>(buf_ + size_, N - size_);
      }
      size_ = N;
    }

    const_reference operator[](size_t i) const { assert(i < size_); return *(buf_ + i); }
    reference operator[](size_t i) { assert(i < size_); return *(buf_ + i); }

    const_reference back() const { assert(size_); return *(buf_ + size_ - 1); }
    reference back() { assert(size_); return *(buf_ + size_ - 1); }


    void pop_back()
    {
      assert (size_ > 0);
      aligned_destruct<value_type>(buf_ + size_ - 1, 1);
      --size_;
    }


    void push_back(value_type const& val)
    {
      if (size_ == capacity_) reserve(capacity_ ? 2 * capacity_ : 1);  // grow by 100%
      aligned_construct<value_type>(buf_ + size_, val);
      ++size_;
    }


    void push_back(value_type&& val)
    {
      if (size_ == capacity_) reserve(capacity_ ? 2 * capacity_ : 1);  // grow by 100%
      aligned_construct<value_type>(buf_ + size_, std::forward<value_type>(val));
      ++size_;
    }


    template <typename... Args>
    void emplace(iterator it, Args&&... args)
    {
      aligned_destruct<value_type>(it, 1);
      ::new ((value_type*)&(char&)(*it)) value_type(std::forward<Args>(args)....);
    }


    template <typename... Args>
    void emplace_back(Args&&... args)
    {                                                                                               
      if (size_ == capacity_) reserve(capacity_ ? 2 * capacity_ : 1);  // grow by 100%
      ::new ((value_type*)&(char&)(*(buf_ + size_))) value_type(std::forward<Args>(args)...);
      ++size_;
    }


    bool empty() const { return 0 != size_; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    iterator begin() { return buf_; }
    iterator end() { return buf_ + size_; }
    const_iterator begin() const { return const_iterator((pointer)&*buf_); }
    const_iterator end() const { return const_iterator((pointer)&*buf_) + size_; }

    
    void swap(aligned_buffer& rhs)
    {
      buf_.swap(rhs.buf_);
      std::swap(size_, rhs.size_);
      std::swap(capacity_, rhs.capacity_);
    }

  private:
    void orphan()
    {
      if (size_)
      {
        aligned_destruct<value_type>(buf_, size_);
        alloc_.deallocate((pointer)&*buf_, capacity_); 
      }
    }

    iterator buf_;
    size_t size_, capacity_;
    allocator_type alloc_;
  };


}

using memory::aligned_buffer;

}


#endif
