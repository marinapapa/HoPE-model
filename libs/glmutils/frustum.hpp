//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup frustum Viewing Frustum
//!
//! \file frustum.hpp Supports Clipping against a Viewing Frustum.
//! \ingroup intersection frustum bbox


#ifndef glmutils_frustum_hpp
#define glmutils_frustum_hpp

#include <glm/gtc/matrix_access.hpp>
#include <glmutils/bbox_plane.hpp>


namespace glmutils {

  using namespace glm;
  using namespace glm::detail;


  //! \brief Viewing frustum
  //!
  //! Provides frustum-bbox intersection test.
  //!
  template<typename T>
  class tfrustum
  {
  public:
    typedef T             value_type;  //!< Underlying numeric type
    typedef typename tvec3<T>     vec_type;
    typedef typename tbbox<vec_type> bbox_type;

    //! Default Constructor: Result is undefined.
    tfrustum() {}

    //! Construction from column-major Projection-Model matrix.
    //! Based on: \n
    //! "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix"
    //! by Gil Gribb & Klaus Hartmann.
    //!
    explicit tfrustum(const tmat4x4<T>& PM) { extract_planes(PM); }

    //! Construction from column-major Model matrix and Projection matrix.
    //! Based on: \n
    //! "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix"
    //! by Gil Gribb & Klaus Hartmann.
    //!
    tfrustum(const tmat4x4<T>& model, const tmat4x4<T>& proj) { extract_planes(proj*model); }

  private:
    void extract_planes(const tmat4x4<T>& PM)
    {
      plane_[0] = -(row(PM,3) + row(PM,0));    // Left clipping plane
      plane_[1] = -(row(PM,3) - row(PM,0));    // Right clipping plane
      plane_[2] = -(row(PM,3) - row(PM,1));    // Top clipping plane
      plane_[3] = -(row(PM,3) + row(PM,1));    // Bottom clipping plane
      plane_[4] = -(row(PM,3) + row(PM,2));    // Near clipping plane
      plane_[5] = -(row(PM,3) - row(PM,2));    // Far clipping plane
      for (int i=0; i<6; ++i)
      {
        iv_[i] = plane_iv(plane_[i]);
      }
    }

  public:
    //! Intersection test against bounding box
    glmutils::intersect_test intersect(const bbox_type& box) const
    {
      bool inside = true;
      for (int i=0; i<6; ++i)
      {
        const intersect_test test = glmutils::intersect(box, plane_[i], iv_[i]);
        if (OUTSIDE == test)
        {
          return OUTSIDE;
        }
        inside = inside && (INSIDE == test);
      }
      return (inside) ? INSIDE : INTERSECTING;
    }

  private:
    tvec4<T>    plane_[6];
    tmat2x3<int>  iv_[6];
  };


  //! Frustum-bbox intersection test.
  //!
  template<typename T>
  inline glmutils::intersect_test intersect(const tfrustum<T>& frustum, const typename tfrustum<T>::bbox_type& box)
  {
    return frustum.intersect(box);
  }

  //! Frustum-bbox inclusion test.
  //!
  template<typename T>
  inline bool is_inside(const tfrustum<T>& frustum, const typename tfrustum<T>::bbox_type& box)
  {
    return INSIDE == intersect(frustum, box);
  }


  //! Frustum-bbox exclusion test.
  //!
  template<typename T>
  inline bool is_outside(const tfrustum<T>& frustum, const typename tfrustum<T>::bbox_type& box)
  {
    return OUTSIDE == intersect(frustum, box);
  }

  //! Viewing frustum of floats.
  typedef tfrustum<float> frustum;


}   // namespace glmutils


#endif  // glmutils_frustum_hpp
