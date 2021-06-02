//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file bbox_plane.hpp Bbox-plane intersection tests.
//! \sa plane
//! \ingroup bbox intersection plane



#ifndef glmutils_bbox_plane_hpp
#define glmutils_bbox_plane_hpp


#include <glm/gtx/integer.hpp>  // mod
#include <glmutils/bbox.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/intersect_test.hpp>


namespace glmutils {

  using namespace glm;
  using namespace glm::detail;


  inline ivec3 ivec3mod(const ivec3& X, int a)
  {
    return ivec3(glm::mod(X.x, a),
           glm::mod(X.y, a),
           glm::mod(X.z, a));
  }


  //! Bbox-plane intersection test.
  //!
  //! Based on Tomas Akinine-Moeller, Eric Haines
  //! Real-time rendering, 2nd Edition, p587-588
  //! \param[in] box The bounding box to test against.
  //! \param[in] plane The plane in constant-normal form encoded as 4-component vector
  //! (x,y,z,d) where (x,y,z) is the normal of the plane and d is its distance to the origin.
  //! \return \a INTERSECTING if we have a intersection, \a OUTSIDE if the box is in the
  //! negative half-space of the plane (same side as the plane normal)
  //! and \a INSIDE if the box is in the positive half-space of the plane
  //! (opposite side of the plane normal).
  //!
  template<typename T>
  inline glmutils::intersect_test intersect(const tbbox<tvec3<T> >& box, const tvec4<T>& plane)
  {
    const ivec3 ivmax( (plane.x >= 0) ? 3 : 0, (plane.y >= 0) ? 4 : 1, (plane.z >= 0) ? 5 : 2 );
    const T* pb = glm::value_ptr(box.p0());
    const tvec3<T> vmax(pb[ivmax.x], pb[ivmax.y], pb[ivmax.z]);
    const tvec3<T> pnorm(plane);
    if ((dot(pnorm, vmax) + plane.w) < 0)
    {
      return INSIDE;
    }
    const ivec3 ivmin(ivec3mod(ivmax+3, 6));
    const tvec3<T> vmin(pb[ivmin.x], pb[ivmin.y], pb[ivmin.z]);
    return ((dot(pnorm, vmin) + plane.w) > 0) ? OUTSIDE : INTERSECTING;
  }


  //! Optimized bbox-plane intersection test.
  //!
  //! Based on Tomas Akinine-Moeller, Eric Haines
  //! Real-time rendering, 2nd Edition, p587-588
  //! \param[in] box The bounding box to test against.
  //! \param[in] plane The plane in constant-normal form encoded as 4-component vector
  //! (x,y,z,d) where (x,y,z) is the normal of the plane and d is its
  //! distance to the origin.
  //! \param[in] iv Precomputed helper matrix, i.e. \a iv == \a plane_iv(plane)
  //! \return \a INTERSECTING if we have a intersection, \a OUTSIDE if the box is in the
  //! negative half-space of the plane (same side as the plane normal)
  //! and \a INSIDE if the box is in the positive half-space of the plane
  //! (opposite side of the plane normal).
  //!
  //! \note Optimized for testing many boxes against the same plane since
  //! \a iv is constant "per plane".
  //!
  //! \sa plane_iv
  //!
  template<typename T>
  inline glmutils::intersect_test intersect(const tbbox<tvec3<T> >& box, const tvec4<T>& plane, const tmat2x3<int>& iv)
  {
    const T* pb = glm::value_ptr(box.p0());
    const tvec3<T> vmax(pb[iv[0].x], pb[iv[0].y], pb[iv[0].z]);
    if ((dot(tvec3<T>(plane), vmax) + plane.w) < 0)
    {
      return INSIDE;
    }
    const tvec3<T> vmin(pb[iv[1].x], pb[iv[1].y], pb[iv[1].z]);
    return ((dot(tvec3<T>(plane), vmin) + plane.w) > 0) ? OUTSIDE : INTERSECTING;
  }


  //! Helper function for the optimized bounding box-plane intersection test.
  //! \return The indices of the diagonal of boxes to test against.
  //!
  template<typename T>
  inline tmat2x3<int> plane_iv(const tvec4<T>& plane)
  {
    const ivec3 ivmax( (plane.x >= 0) ? 3 : 0, (plane.y >= 0) ? 4 : 1, (plane.z >= 0) ? 5 : 2 );
    tmat2x3<int> tmp(ivmax, ivec3mod(ivmax + 3, 6));
    return tmp;
  }



}   // namespace glmutils


#endif  // glmutils_bbox_plane_hpp
