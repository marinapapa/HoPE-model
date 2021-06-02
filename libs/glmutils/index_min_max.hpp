//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file index_min_max.hpp Index based min/max and ranking.
//! \ingroup misc

#ifndef glmutils_index_min_max_hpp
#define glmutils_index_min_max_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Returns the index of the smallest element of \c v
  template<typename T>
  inline int index_min(const tvec2<T>& v)
  {
    return (v.x < v.y) ? 0 : 1;
  }


  //! Returns the index of the smallest element of \c v
  template<typename V>
  inline int index_min(const V& v)
  {
    int m = 0;
    for (int i=1; i<trait<V>::size; ++i)
    {
      if (v[m] > v[i]) m = i;
    }
    return m;
  }


  //! Returns the index of the biggest element of \c v
  template<typename T>
  inline int index_max(const tvec2<T>& v)
  {
    return (v.x > v.y) ? 0 : 1;
  }


  //! Returns the index of the biggest element of \c v
  template<typename V>
  inline int index_max(const V& v)
  {
    int m = 0;
    for (int i=1; i<trait<V>::size; ++i)
    {
      if (v[m] < v[i]) m = i;
    }
    return m;
  }


  //! Returns (index of the smallest element of \c v, index of the biggest element of \c v)
  //! or (0,1) if all elements are equal
  template<typename T>
  inline tvec2<int> index_min_max(const tvec2<T>& v)
  {
    return (v.x <= v.y) ? tvec2<int>(0,1) : tvec2<int>(1,0);
  }


  //! Returns (index of the smallest element of \c v, index of the biggest element of \c v)
  //! or (0,2) if all elements are equal
  template<typename T>
  inline tvec2<int> index_min_max(const tvec3<T>& v)
  {
    int x = 0;
    int y = 1;
    int z = 2;
    if (v[0] > v[2]) { x = 2; z = 0; }
    if (v[1] > v[z]) { y = z; z = 1; }
    if (v[x] > v[y]) { x = y; }
    return tvec2<int>(x,z);
  }


  //! Returns (index of the smallest element of \c v, index of the biggest element of \c v)
  //! or (0,V::size-1) if all elements are equal
  template<typename V>
  inline tvec2<int> index_min_max(const V& v)
  {
    tvec2<int> m(0,0);
    for (int i=1; i<trait<V>::size; ++i)
    {
      if (v[m.x] > v[i]) { m.x = i; }
      if (v[m.y] <= v[i]) { m.y = i; }
    }
    return m;
  }

  //! Returns the index rank of the elements in \c v
  template<typename T>
  inline tvec2<int> index_rank(const tvec2<T>& v)
  {
    return index_min_max(v);
  }


  //! Returns the index rank of the elements in \c v
  template<typename T>
  inline tvec3<int> index_rank(const tvec3<T>& v)
  {
    tvec3<int> rank(0,1,2);
    if (v[0] > v[2]) { rank.x = 2; rank.z = 0; }
    if (v[1] > v[rank.z]) { rank.y = rank.z; rank.z = 1; }
    if (v[rank.x] > v[rank.y]) { int tmp = rank.x; rank.x = rank.y; rank.y = tmp; }
    return rank;
  }


  //! Returns the index rank of the elements in \c v
  template<typename T>
  inline tvec4<int> index_rank(const tvec4<T>& v)
  {
    tvec4<int> r(0,1,2,3);
    // bubble sort
    for (int p=0; p<3; ++p)
    {
      for (int i=p+1; i<4; ++i)
      {
        if (v[r[p]] > v[r[i]]) { int tmp = r[p]; r[p] = r[i]; r[i] = tmp; }
      }
    }
    return r;
  }


  //! Returns the index rank of the elements in \c v
  template<typename V>
  inline typename trait<V>::template other<int>::type index_rank(const V& v)
  {
    typename trait<V>::template other<int>::type r;
    for(int i=0; i<trait<V>::size; ++i) r[i] = i;
    // bubble sort
    for (int p=0; p<trait<V>::size-1; ++p)
    {
      for (int i=p+1; i<trait<V>::size; ++i)
      {
        if (v[r[p]] > v[r[i]]) { int tmp = r[p]; r[p] = r[i]; r[i] = tmp; }
      }
    }
    return r;
  }


}  // namespace glmutils


#endif  // glmutils_index_mim_max_hpp
