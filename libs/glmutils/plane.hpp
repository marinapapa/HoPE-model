//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup plane Planes
//! \file plane.hpp Support for planes in constant-normal form (implicit)
//!
//! The constant-normal form of a plane represented by a 4-vector
//! \f[ P = (n_x, n_y, n_z, d) \f]
//! where \f$ (n_x, n_y, n_z) \f$ is the normal of the plane and
//! \f$d\f$ the distance to the origin.
//!
//! \ingroup plane


#ifndef glmutils_plane_hpp
#define glmutils_plane_hpp


#include <limits>
#include <glm/glm.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Returns the plane in constant-normal form given 3 points on the plane
  //!
  template<typename T>
  inline tvec4<T> implicitPlane(const tvec3<T>& u, const tvec3<T>& v, const tvec3<T>& w)
  {
    const tvec3<T> n( normalize( cross(u-w, v-w) ) );
    const T d = - dot(n,u);
    return tvec4<T>(n, d);
  }


  //! Returns the signed distance point to plane
  //!
  //! Interpretation of the sign of the returned value: \n
  //! returned value == 0 : the point lies on plane. \n
  //! returned value > 0 : the point lies on the same side of the plane as the point q + pn; q on plane. \n
  //! returned value < 0 : the point lies on the same side of the plane as the point q - pn; q on plane. \n
  //!
  template<typename T>
  inline T signedDistance(const tvec3<T>& point, const tvec4<T>& plane)
  {
    return dot(tvec3<T>(plane), point) + plane.w;
  }


  //! Returns the signed distance point to plane given three point on the plane
  //!
  //! Interpretation of the sign of the returned value: \n
  //! returned value == 0 : the point lies on plane. \n
  //! returned value > 0 : the point lies on the same side of the plane as the point q + pn; q on plane. \n
  //! returned value < 0 : the point lies on the same side of the plane as the point q - pn; q on plane. \n
  //!
  template<typename T>
  inline T signedDistance(const tvec3<T>& point, const tvec3<T>& u, const tvec3<T>& v, const tvec3<T>& w)
  {
    return signedDistance(point, implicitPlane(u,v,w));
  }


  //! Project the point p to the plane
  //!
  template<typename T>
  inline tvec3<T> projectToPlane(const tvec3<T>& p, const tvec4<T>& plane)
  {
    tvec3<T> norm(plane);
    return tvec3<T>( p - norm * dot(p, norm) );
  }


  //! Project the point p to the plane given by its normal
  //!
  template<typename T>
  inline tvec3<T> projectToPlane(const tvec3<T>& p, const tvec3<T>& normal)
  {
    return tvec3<T>( p - normal * dot(p, normal) );
  }


}   // namespace glmutils


#endif  // glmutils_plane_hpp
