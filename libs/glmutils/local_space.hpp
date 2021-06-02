//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup local Local coordinate system
//! \file local_space.hpp Support for local coordinate systems in 2D and 3D.
//! \ingroup local


#ifndef glmutils_local_space_hpp
#define glmutils_local_space_hpp


#include <glm/glm.hpp>


namespace glmutils {

  using namespace glm;
  using namespace glm::detail;



  //! Returns 2D transformation matrix into local space given as x,y vectors.
  template<typename T>
  inline tmat2x2<T> localSpace(const tvec2<T>& x, const tvec2<T>& y)
  {
    return tmat2x2<T>(x, y);
  }


  //! Returns 3D transformation matrix into local space given as forward,up,side vectors.
  template<typename T>
  inline tmat3x3<T> localSpace(const tvec3<T>& forward, const tvec3<T>& up, const tvec3<T>& side)
  {
    return tmat3x3<T>(forward, up, side);
  }


  //! Returns 3D homogeneous transformation matrix into local space given as forward,up,side and orig vectors.
  template<typename T>
  inline tmat4x4<T> localSpace(const tvec3<T>& forward, const tvec3<T>& up, const tvec3<T>& side, const tvec3<T>& orig)
  {
    return tmat4x4<T>(tvec4<T>(forward, 0), tvec4<T>(up, 0), tvec4<T>(side, 0), tvec4<T>(orig, 1));
  }


  //! Transforms the 2D vector \a v into local space given as x,y vectors.
  template<typename T>
  inline tvec2<T> vectorLocalSpace(const tvec3<T>& v,
                   const tvec3<T>& x,
                   const tvec3<T>& y)
  {
    return tvec2<T>( dot(x, v), dot(y, v) );
  }


  //! Transforms the 3D vector \c v into local space given as forward,up,side vectors.
  template<typename T>
  inline tvec3<T> vectorLocalSpace(const tvec3<T>& v,
                   const tvec3<T>& forward,
                   const tvec3<T>& up,
                   const tvec3<T>& side)
  {
    return tvec3<T>( dot(forward, v), dot(up, v), dot(side, v) );
  }


  //! Transforms the 2D vector v into local space given as transformation matrix \c L.
  template<typename T>
  inline tvec2<T> vectorLocalSpace(const tvec2<T>& v,
                   const tmat2x2<T>& L)
  {
    return L * v;
  }


  //! Transforms the 2D vector v into local space given as transformation matrix \c L.
  template<typename T>
  inline tvec3<T> vectorLocalSpace(const tvec3<T>& v,
                   const tmat3x3<T>& L)
  {
    return L * v;
  }


  //! Transforms the 2D point \c p into local space given as x,y and orig vectors.
  template<typename T>
  inline tvec2<T> pointLocalSpace(const tvec2<T>& p,
                  const tvec2<T>& x,
                  const tvec2<T>& y,
                  const tvec2<T>& orig)
  {
    return vectorLocalSpace<T>(p - orig, x, y) + orig;
  }


  //! Transforms the 3D point \c p into local space given as forward,up,side and orig vectors.
  template<typename T>
  inline tvec3<T> pointLocalSpace(const tvec3<T>& p,
                  const tvec3<T>& forward,
                  const tvec3<T>& up,
                  const tvec3<T>& side,
                  const tvec3<T>& orig)
  {
    return vectorLocalSpace<T>(p - orig, forward, up, side) + orig;
  }


  //! Transforms the 2D point \c p into local space given as transformation matrix \c L.
  template<typename T>
  inline tvec2<T> pointLocalSpace(const tvec2<T>& p,
                  const tmat2x2<T>& L,
                  const tvec2<T>& orig)
  {
    return (L * (p-orig)) + orig;
  }


  //! Transforms the 3D point \c p into local space given as transformation matrix \c L.
  template<typename T>
  inline tvec3<T> pointLocalSpace(const tvec3<T>& p,
                  const tmat3x3<T>& L,
                  const tvec3<T>& orig)
  {
    return (L * (p-orig)) + orig;
  }


  //! Returns the spherical coordinates (longitude [deg], latitude [deg], r)
  //! of the point \c v in local space given as forward,up,side vectors.
  //!
  template<typename T>
  inline tvec3<T> sphericalLocalSpace(const tvec3<T>& v, const tvec3<T>& forward, const tvec3<T>& up, const tvec3<T>& side)
  {
    const tvec3<T> local = vectorLocalSpace<T>(v, forward, up, side);
    const T f = local.x;
    const T u = local.y;
    const T s = local.z;
    tvec3<T> tmp( degrees(atan2(s,f)),
            degrees(atan2(u, std::sqrt(f*f + s*s))),
            length<T>(local) );
    return tmp;
  }


  //! Returns the spherical coordinates (longitude [deg], latitude [deg], r)
  //! of the point \c v in local space given as transformation matrix \c L.
  //!
  template<typename T>
  inline tvec3<T> sphericalLocalSpace(const tvec3<T>& v, const tmat3x3<T>& L)
  {
    const tvec3<T> local = vectorLocalSpace<T>(v, L);
    const T f = local.x;
    const T u = local.y;
    const T s = local.z;
    tvec3<T> tmp( degrees(atan2(s,f)),
            degrees(atan2(u, std::sqrt(f*f + s*s))),
            length<T>(local) );
    return tmp;
  }


}   // namespace glmutils


#endif  // glmutils_local_space_hpp
