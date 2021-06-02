//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file bbox_bbox.hpp Bbox-bbox intersection tests.
//! \ingroup bbox intersection


#ifndef glmutils_bbox_bbox_hpp
#define glmutils_bbox_bbox_hpp


#include <glmutils/bbox.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/intersect_test.hpp>


namespace glmutils {

  using namespace glm;
  using namespace glm::detail;


  //! Returns true if the bounding box \a b and the point \a p are disjoint.
  template<typename bbox>
  inline bool disjoint(const bbox& a, const typename bbox::vec_type& p)
  {
    for (int i=0; i<trait<typename bbox::vec_type>::size; ++i)
    {
      if (a.p1()[i] < p[i] || a.p0()[i] > p[i])
      {
        return true;
      }
    }
    return false;
  }


  //! Returns true if the bounding boxes \a a and \a b are disjoint.
  template<typename bbox>
  inline bool disjoint(const bbox& a, const bbox& b)
  {
    for (int i=0; i<trait<typename bbox::vec_type>::size; ++i)
    {
      if (a.p1()[i] < b.p0()[i] || a.p0()[i] > b.p1()[i])
      {
        return true;
      }
    }
    return false;
  }


  //! Returns true if the bounding box \a a contains the point \a p.
  template<typename bbox>
  inline bool contains(const bbox& a, const typename bbox::vec_type& p)
  {
    return !disjoint(a, p);
  }


  //! Returns true if the bounding box \a a contains the bounding box \a b.
  template<typename bbox>
  inline bool contains(const bbox& a, const bbox& b)
  {
    return !( disjoint(a,b.p0()) || disjoint(a,b.p1()) );
  }


  //! Bbox-bbox intersection test.
  template<typename bbox>
  inline glmutils::intersect_test intersect(const bbox& a, const bbox& b)
  {
    bool inside = true;
    for (int i=0; i<trait<typename bbox::vec_type>::size; ++i)
    {
      bool t1 = a.p0()[i] > b.p1()[i];
      bool t2 = b.p0()[i] > a.p1()[i];
      if ( t1 || t2 )
      {
        return OUTSIDE;
      }
      inside = inside && ( !(t1 || t2) );
    }
    return (inside) ? INSIDE : INTERSECTING;
  }


  //! \returns the squared distance between the point \a p and the box \c a, and zero if \a p is inside the box.
  //! \sa distance()
  template<typename bbox>
  typename bbox::value_type squaredDistance(const bbox& a, const typename bbox::vec_type& p)
  {
    typedef typename bbox::value_type scalar;
    typedef typename bbox::vec_type vec;
    scalar dist2 = 0;
    scalar aux;
    for (int k=0; k<trait<vec>::size; ++k)
    {
      if( a.p0()[k] > p[k] )
      {
        aux = a.p0()[k] - p[k];
        dist2 += aux*aux;
      }
      else if( p[k] > a.p1()[k] )
      {
        aux = p[k] - a.p1()[k];
        dist2 += aux*aux;
      }
    }
    return dist2;
  }


  //! \returns the squared distance between the box \a b and the box \c a, and zero if \a a intersects \a b.
  //! \sa distance()
  template<typename bbox>
  typename bbox::value_type squaredDistance(const bbox& a, const bbox& b)
  {
    typedef typename bbox::value_type scalar;
    typedef typename bbox::vec_type vec;
    scalar dist2 = 0;
    scalar aux;
    for (int k=0; k<trait<vec>::size; ++k)
    {
      if( a.p0()[k] > b.p1()[k] )
      {
        aux = a.p0()[k] - b.p1()[k];
        dist2 += aux*aux;
      }
      else if( b.p0()[k] > a.p1()[k] )
      {
        aux = b.p0()[k] - a.p1()[k];
        dist2 += aux*aux;
      }
    }
    return dist2;
  }


  //! \returns the squared distance between the point \a p and the box \c a, and zero if \a p is inside the box.
  //! \sa distance()
  template<typename bbox>
  typename bbox::value_type distance(const bbox& a, const typename bbox::vec_type& p)
  {
    return std::sqrt(squaredDistance(a,p));
  }


  //! \returns the squared distance between the box \a b and the box \c a, and zero if \a a intersects \a b.
  //! \sa distance()
  template<typename bbox>
  typename bbox::value_type distance(const bbox& a, const bbox& b)
  {
    return std::sqrt(squaredDistance(a,b));
  }

}  // namespace glmutils


#endif  // glmutils_bbox_bbox_hpp
