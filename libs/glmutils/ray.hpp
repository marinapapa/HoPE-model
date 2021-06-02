//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup ray Rays
//! Ray-Object intersection tests.
//! Rays are represented as \f$ \mathbf{r} = \mathbf{orig} + t \, \mathbf{dir}; \quad t \ge 0 \f$ \n
//! Lines are represented as rays with \f$ -\infty \le t \le +\infty \f$ \n
//! Line segments are represented as rays with restricted parameter range or as
//! as set of two points.
//!
//! \file ray.hpp Basic functions involving a ray.
//! \ingroup ray intersection plane


#ifndef glmutils_ray_hpp
#define glmutils_ray_hpp


#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/perp_dot.hpp>
#include <glmutils/plane.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Returns the closest squared distance of a point to a ray or line.
  template<typename V>
  inline typename trait<V>::value_type distanceSqRayPoint(const V& orig,
                              const V& dir,
                              const V& point)
  {
    const V d = point - orig;
    const typename trait<V>::value_type t0 = dot(dir, d);
    const V w = d - (dir * t0);
    return length2(w);
  }


  //! Ray-ray (line-line) intersection 2D \n w solution for ray parameters s and t
  //!
  //! Solving \f$ \mathbf{o_A} + s \, \mathbf{d_A} = \mathbf{o_A} + t \, \mathbf{d_A} \f$
  //! for s and t.
  //!
  //! \param[in] origA Origin of ray A
  //! \param[in] dirA Direction of ray A
  //! \param[in] origB Origin of ray B
  //! \param[in] dirB Direction of ray B
  //! \param[out] s solution for s
  //! \param[out] t solution for t
  //! \return \c false if the the lines are (nearly) parallel. In this
  //! case s and t are undefined (for obvious reasons).
  //!
  //! \attention Despite the name, this is a line-line intersection test.
  //! Test against \c s > 0, t > 0 if you are interested in rays...
  //!
  template<typename T>
  inline bool intersectRayRay(const tvec2<T>& origA,
                const tvec2<T>& dirA,
                const tvec2<T>& origB,
                const tvec2<T>& dirB,
                T& s, T& t)
  {
    const T denom = perpDot(dirA, dirB);
    if (abs(denom) < T(0.0000001))
    {
      return false;
    }
    const T rdenom = T(1) / denom;
    // hm, strange: this is not slower than to exploit the identity
    // perpDot(a-b,c)==-perpDot(b-a,c):
    // (relates to intel icl 11.0, msvc9)
    s = perpDot(origB-origA, dirB) * rdenom;
    t = perpDot(origA-origB, dirA) * -rdenom;
    return true;
  }


  //! Ray-ray (line-line) intersection 3D \n w solution for ray parameters s and t
  //!
  //! Solving \f$ \mathbf{o_A} + s \, \mathbf{d_A} = \mathbf{o_A} + t \, \mathbf{d_A} \f$
  //! for s and t. \n\n
  //! \param[in] origA Origin of ray A
  //! \param[in] dirA Direction of ray A
  //! \param[in] origB Origin of ray B
  //! \param[in] dirB Direction of ray B
  //! \param[out] s solution for s
  //! \param[out] t solution for t
  //! \return \c false if the the lines are (nearly) parallel. In this
  //! case \b s and \b t are undefined (for obvious reasons).
  //!
  //! \par Why most non-intersection are not rejected
  //! Two rays in 3D can only intersect if they are coplanar - say very rarely.
  //! For this, this function returns \c false only if the two rays are
  //! (nearly) parallel. In all other cases the returned values for \c s and \c t
  //! are still meaningful even if the rays don't intersect.\n
  //! The interpretation is as follows: The two points
  //! \f[ \mathbf{P_A} = \mathbf{o_A} + s \, \mathbf{d_A} \f]
  //! \f[ \mathbf{P_B} = \mathbf{o_B} + t \, \mathbf{d_B} \f]
  //! are building the line segment \f$ \overline{\mathbf{P_A P_B}} \f$
  //! that is perpendicular to ray A and perpendicular to ray B, hence
  //! we have found the two points of the \b closest \b approach
  //! of the two rays - they can't become any closer. A real intersection
  //! could be detected by checking the distance between these points
  //! against zero.
  //!
  //! \attention Despite the name, this is a line-line intersection test.
  //! Test against \c s > 0, t > 0 if you are interested in rays...
  //!
  template<typename T>
  inline bool intersectRayRay(const tvec3<T>& origA,
                const tvec3<T>& dirA,
                const tvec3<T>& origB,
                const tvec3<T>& dirB,
                T& s, T& t)
  {
    const tvec3<T> C( cross(dirA, dirB) );
    const T denom = length2(C);
    if (denom < T(0.0000001))
    {
      return false;  // nearly parallel
    }
    const tvec3<T> D(origB - origA);
    const tvec3<T> S( cross(D, dirB) );
    T rdenom = T(1) / denom;
    s = dot(S, C) * rdenom;
    const tvec3<T> X( cross(D, dirA) );
    t = dot(X, C) * rdenom;
    return true;
  }


  //! Ray-sphere intersection test.
  template<typename T>
  inline bool intersectRaySphere(const tvec3<T>& orig,
                   const tvec3<T>& dir,
                   const tvec3<T>& center,
                   T radius)
  {
    const tvec3<T> d(center - orig);
    const T rr = radius*radius;
    const T s = dot(d, dir);
    const T dd = length2(d);
    if ((s < T(0) && dd > rr) || (dd-s*s > rr))
    {
      return false;      // behind and not inside
    }
    return true;
  }


  //! Ray-sphere intersection test \n w calculation of intersection position and normal to the sphere.
  template<typename T>
  inline bool intersectRaySphere(const tvec3<T>& orig,
                   const tvec3<T>& dir,
                   const tvec3<T>& center,
                   T radius,
                   tvec3<T>& position,
                   tvec3<T>& norm)
  {
    const tvec3<T> d(center - orig);
    const T rr = radius*radius;
    const T s = dot(d, dir);
    const T dd = length2(d);
    T mm;
    if ((s < T(0) && dd > rr) || ((mm = dd-s*s) > rr))
    {
      return false;      // behind and not inside
    }
    const T q = std::sqrt(rr - mm);  // r^2 = q^2 + m^2
    const T t = (dd > rr) ? s-q : s+q;
    position = t * dir + orig;
    norm = (position - center) / radius;
    return true;
  }


  //! Ray-circle intersection test.
  template<typename T>
  inline bool intersectRayCircle(const tvec2<T>& orig,
                   const tvec2<T>& dir,
                   const tvec2<T>& center,
                   T radius)
  {
    const tvec2<T> d(center - orig);
    const T rr = radius*radius;
    const T s = dot(d, dir);
    const T dd = length2(d);
    if ((s < T(0) && dd > rr) || (dd-s*s > rr))
    {
      return false;      // behind and not inside
    }
    return true;
  }


  //! Ray-circle intersection test \n w calculation of intersection position and normal to the sphere.
  template<typename T>
  inline bool intersectRayCircle(const tvec2<T>& orig,
                   const tvec2<T>& dir,
                   const tvec2<T>& center,
                   T radius,
                   tvec2<T>& position,
                   tvec2<T>& norm)
  {
    const tvec2<T> d(center - orig);
    const T rr = radius*radius;
    const T s = dot(d, dir);
    const T dd = length2(d);
    T mm;
    if ((s < T(0) && dd > rr) || ((mm = dd-s*s) > rr))
    {
      return false;      // behind and not inside
    }
    const T q = std::sqrt(rr - mm);  // r^2 = q^2 + m^2
    const T t = (dd > rr) ? s-q : s+q;
    position = t * dir + orig;
    norm = (position - center) / radius;
    return true;
  }


  //! Ray-Plane intersection test \n w solution for ray-parameter t.
  template<typename T>
  inline bool intersectRayPlane(const tvec3<T>& orig,
                  const tvec3<T>& dir,
                  const tvec4<T>& plane,
                  T& t)
  {
    tvec3<T> norm(plane);
    const T denom = dot(norm, dir);
    if (denom < std::numeric_limits<T>::epsilon())
    {
      return false;    // back facing or parallel
    }
    t = (-plane.w - dot(norm, orig)) / denom;
    return (t >= T(0));
  }


  //! Line-Plane intersection test \n w solution for ray-parameter t.
  template<typename T>
  inline bool intersectLinePlane(const tvec3<T>& orig,
                   const tvec3<T>& dir,
                   const tvec4<T>& plane,
                   T& t)
  {
    tvec3<T> norm(plane);
    const T denom = dot(norm, dir);
    if (abs(denom) < std::numeric_limits<T>::epsilon())
    {
      return false;    // parallel
    }
    t = (-plane.w - dot(norm, orig)) / denom;
    return true;
  }


  //! Line segment - Plane intersection test \n w solution for ray-parameter t.
  template<typename T>
  inline bool intersectLineSegmentPlane(const tvec3<T>& p0,
                      const tvec3<T>& p1,
                      const tvec4<T>& plane,
                      T& t)
  {
    const bool hit = intersectLinePlane(p0, p1-p0, plane, t);
    return (hit && t >= T(0) && t <= T(1));
  }


}   // namespace glmutils


#endif  // glmutils_ray_hpp
