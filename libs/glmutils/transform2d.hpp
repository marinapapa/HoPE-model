//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup transform Matrix transformations
//! \file transform2d.hpp 2D Transformation matrics
//! \ingroup homogeneous transform


#ifndef glmutils_transform2d_hpp
#define glmutils_transform2d_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Returns a homogeneous 2D translation matrix.
  template <typename T>
  inline tmat3x3<T> translate(const tmat3x3<T>& m, T x, T y)
  {
    tmat3x3<T> r(m);
    r[2][0] = m[0][0] * x + m[1][0] * y + m[2][0];
    r[2][1] = m[0][1] * x + m[1][1] * y + m[2][1];
    r[2][2] = m[0][2] * x + m[1][2] * y + m[2][2];
    return r;
  }


  //! Returns a homogeneous 2D translation matrix.
  template <typename T>
  inline tmat3x3<T> translate(const tmat3x3<T>& m, const tvec2<T>& v)
  {
    return glmutils::translate(m, v.x, v.y);
  }


  //! Returns a homogeneous 2D rotation matrix.
  template <typename T>
  inline tmat3x3<T> rotate(const tmat3x3<T>& m, T angle)
  {
    T a = radians(angle);
    T c = std::cos(a);
    T s = std::sin(a);
    return m * tmat3x3<T>(tvec3<T>(c,-s,0), tvec3<T>(s,c,0), tvec3<T>(0,0,1));
  }


  //! Returns a homogeneous 2D scaling matrix.
  template <typename T>
  inline tmat3x3<T> scale(const tmat3x3<T>& m, T x, T y)
  {
    tmat3x3<T> r;
    r[0] = m[0] * x;
    r[1] = m[1] * y;
    r[2] = m[2];
    return r;
  }


  //! Returns a homogeneous 2D scaling matrix.
  template <typename T>
  inline tmat3x3<T> scale(const tmat3x3<T>& m, const tvec2<T>& v)
  {
    return glmutils::scale(m, v.x, v.y);
  }

}   // namespace glmutils


#endif  // glmutils_transform2d_hpp
