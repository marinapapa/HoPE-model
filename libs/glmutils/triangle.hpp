//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup triangle Triangles
//! \file triangle.hpp Ray-triangle intersection \n Point in triangle
//! \ingroup poly intersection triangle ray


#ifndef glmutils_triangle_hpp
#define glmutils_triangle_hpp


#include <limits>
#include <glm/glm.hpp>
#include <glmutils/ray.hpp>
#include <glmutils/polygon.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Ray triangle intersection test
  //!
  //! \return (b,u,v,t) \n
  //! with b = 1|0 (Ray intersects triangle) \n
  //! (u,v): the barycentric coordinates of the intersection point \n
  //! t: solution for the ray parameter t, see \ref ray
  //
  template<typename T>
  tvec4<T> intersectRayTri(const tvec3<T>& orig, const tvec3<T>& dir,
               const tvec3<T>& v0, const tvec3<T>& v1, const tvec3<T>& v2)
  {
    tvec4<T> res(0);
    const tvec3<T> e1(v1 - v0);
    const tvec3<T> e2(v2 - v0);
    const tvec3<T> p(cross(dir, e2));
    const T a = dot(e1, p);
    if (abs(a) < std::numeric_limits<T>::epsilon())
    {
      return res;
    }
    const T f = T(1)/a;
    const tvec3<T> s(orig - v0);
    res.y = f * dot(s, p);      // u
    if (res.y < T(0) || res.y > T(1))
    {
      return res;
    }
    const tvec3<T> q(cross(s, e1));
    res.z = f * dot(dir, q);    // v
    if ((res.z < T(0)) || (res.y + res.z > T(1)))
    {
      return res;
    }
    res.w = f * dot(e2, q);
    res.x = T(1);
    return res;
  }


  //! Point in triangle test in 2D
  //! \return \c true if the point is inside the triangle
  template<typename T>
  inline bool pointInTriangle(const tvec2<T>& p,
                const tvec2<T>& v0, const tvec2<T>& v1, const tvec2<T>& v2)
  {
    bool inside = false;
    bool y0 = (v0[1] >= p[1]);
    bool y1 = (v1[1] >= p[1]);
    if ((y0 != y1) && (((v1[1]-p[1])*(v0[0]-v1[0]) >= (v1[0]-p[0])*(v0[1]-v1[1])) == y1))
    {
      inside = !inside;
    }
    bool y2 = (v2[1] >= p[1]);
    if ((y1 != y2) && (((v2[1]-p[1])*(v1[0]-v2[0]) >= (v2[0]-p[0])*(v1[1]-v2[1])) == y2))
    {
      inside = !inside;
    }
    if ((y0 != y2) && (((v2[1]-p[1])*(v0[0]-v2[0]) >= (v2[0]-p[0])*(v0[1]-v2[1])) == y2))
    {
      inside = !inside;
    }
    return inside;
  }


  //! Point in triangle test in 3D
  //! \return \c true if the point is inside the triangle
  template<typename T, typename IT>
  inline bool pointInTriangle(const tvec3<T>& p,
                const tvec3<T>& v0, const tvec3<T>& v1, const tvec3<T>& v2)
  {
    tvec3<T> v = { v0, v1, v2 };
    return pointInPolygon(p, 3, v);
  }


}   // namespace glmutils


#endif  // glmutils_triangle_hpp
