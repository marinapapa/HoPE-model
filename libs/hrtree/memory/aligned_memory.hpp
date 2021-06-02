// hrtree/memory/aligned_memory.hpp header file
//
// Part of the Hilbert Rtree library.
// Copyright (c) 2000-2014 Hanno Hildenbrandt
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.


#ifndef HRTREE_MEMORY_ALIGNED_MEMORY_INCLUDED
#define HRTREE_MEMORY_ALIGNED_MEMORY_INCLUDED

#include <malloc.h>
#include <exception>
#include <type_traits>
#include <hrtree/config.hpp>


namespace hrtree { namespace memory {


  template <size_t ALIGN>
  void* aligned_malloc(size_t size);

  template <size_t ALIGN>
  void aligned_free(void* ptr);


  template <
    typename T, 
    size_t ALIGN = HRTREE_ALIGNOF(T)
  >
  class aligned_allocator
  {
  public:
    typedef std::true_type allocator_has_align;

    static const size_t align = ALIGN;
    static const size_t aligned_size = (sizeof(T) + (align - 1)) & ~(align - 1);

    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef void*       void_pointer;
    typedef const void* const_void_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T           value_type;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;

    aligned_allocator() {}
    aligned_allocator(const aligned_allocator<T, align >&) {}
    
    template<typename Other>
    aligned_allocator(const aligned_allocator<Other, align >&) {}

    aligned_allocator(const aligned_allocator<T, align >&&) {}

    template<typename Other>
    aligned_allocator(const aligned_allocator<Other, align >&&) {}

    template<typename Other>
    aligned_allocator<T,align >& operator=(const aligned_allocator<Other, align >&)
    {
      return (*this);
    }

    template<class U>
    struct rebind
    {
      typedef aligned_allocator<U, align> other;
    };

    pointer address(reference val) const
    {
      return ((pointer) &(char&)val);
    }

    const_pointer address(const_reference val) const
    {
      return ((const pointer) &(const char&)val);
    }

    size_type max_size() const
    {
      const size_t count = (size_t)(-1) / sizeof (T);
      return (0 < count ? count : 1);                    // stupid hack
    }

    void deallocate(pointer ptr, size_type count)
    {
      aligned_free<align>((void*)ptr);
    }

    pointer allocate(size_type count)
    {
      void_pointer ptr = aligned_malloc<align>(count * aligned_size);
      if (0 == ptr) 
        throw(std::bad_alloc());
      return (pointer)ptr;
    }

    pointer allocate(size_type count, const void*)
    {
      return allocate(count);
    }


    void construct(pointer p, const T& value)
    {
      ::new((void_pointer)p) T(value);
    }


    void construct(pointer p, T&& value)
    {
      ::new ((void_pointer)p) T(std::forward<T>(value));
    }


    void destroy(pointer ptr)
    {
      ptr->~T();
    }

    template <typename U, size_t B>
    bool operator==(const aligned_allocator<U, B>& ) const
    { 
      return align == B; 
    }

    template <typename U, size_t B>
    bool operator!=(const aligned_allocator<U, B>& ) const
    { 
      return align != B; 
    }

  };


  template <size_t ALIGN> 
  inline void* aligned_malloc(size_t size)
  {
    if (ALIGN <= HRTREE_MALLOC_ALIGN)
      return malloc(size);

    const size_t align = std::max<size_t>(HRTREE_ALIGNOF(void*), ALIGN);
    void* raw = malloc(size + align);
    if (0 == raw) return 0;
    void *aligned = reinterpret_cast<void*>( (reinterpret_cast<size_t>(raw) & ~(align - 1)) + align);
    *(reinterpret_cast<void**>(aligned) - 1) = raw;
    return aligned;
  }


  template <size_t ALIGN>
  inline void aligned_free(void* ptr)
  {
    if (ptr)
    { 
      if (ALIGN <= HRTREE_MALLOC_ALIGN) 
        free(ptr);
      else
        free(*(reinterpret_cast<void**>(ptr) - 1));
    }  
  }


  template <typename A, typename enable = void>
  struct allocator_has_align
  {
    typedef std::false_type type;
    static const size_t value = HRTREE_ALIGNOF(typename A::value_type);
  };

  template <typename A>
  struct allocator_has_align<A, typename std::enable_if< A::allocator_has_align::value >::type > 
  { 
    typedef std::true_type type;
    static const size_t value = A::align;
  };


  template <typename T, typename RaIt>
  typename std::enable_if< std::is_trivially_constructible<T>::value >::type 
  inline aligned_default_construct(RaIt, size_t)
  {
  }

  template <typename T, typename RaIt>
  typename std::enable_if< ! std::is_trivially_constructible<T>::value >::type 
  inline aligned_default_construct(RaIt it, const size_t N)
  {
#ifdef HRTREE_PARALLEL_ALIGNED_CONSTRUCT
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel for if(N>500) firstprivate(it) schedule(static) num_threads(numt)
#endif
    for (int i=0; i<(int)N; ++i)
      ::new((T*)&(char&)(*(it + i))) T();
  }

  template <typename T, typename RaIt>
  inline void aligned_construct(RaIt it, const T& val)
  {
    ::new((T*)&(char&)(*(it))) T(val);
  }

  template <typename T, typename RaIt>
  inline void aligned_construct(RaIt it, const size_t N, const T& val)
  {
#ifdef HRTREE_PARALLEL_ALIGNED_CONSTRUCT
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel if(N>500) for firstprivate(it) schedule(static) num_threads(numt)
#endif
    for (int i=0; i<(int)N; ++i)
      ::new((T*)&(char&)(*(it + i))) T(val);
  }

  template <typename T, typename RaIt, typename FwdIt>
  inline void  aligned_construct_iter(RaIt lhs, const size_t N, FwdIt rhs)
  {
#ifdef HRTREE_PARALLEL_ALIGNED_CONSTRUCT
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel if(N > 500) for firstprivate(lhs, rhs) schedule(static) num_threads(numt)
#endif
    for (int i=0; i<(int)N; ++i)
      ::new((T*)&(char&)(*(lhs + i))) T(*(rhs + i));
  }
  

  template <typename T, typename RaIt>
  inline typename std::enable_if< std::is_trivially_destructible<T>::value >::type 
  aligned_destruct(RaIt it, const size_t N)
  {
  }


  template <typename T, typename RaIt>
  inline typename std::enable_if< ! std::is_trivially_destructible<T>::value >::type 
  aligned_destruct(RaIt it, const size_t N)
  {
#ifdef HRTREE_PARALLEL_ALIGNED_CONSTRUCT
    const int numt = hrtree_max_num_threads();
#   pragma omp parallel for if(N>500) firstprivate(it) schedule(static) num_threads(numt)
#endif
    for (int i=0; i<(int)N; ++i)
      (it + i)->~T();
  }

}

using memory::aligned_allocator;

}

#endif
