//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup io IO streaming
//! Lookup will fail without \c "using namespace glm::utils".
//! \file istream.hpp \a istream operators for vector and matrix types.
//! \ingroup io


#ifndef glmutils_istream_hpp
#define glmutils_istream_hpp


#include <iostream>
#include <glm/glm.hpp>


namespace std {

  template<typename T>
  std::istream& operator >> (std::istream& is, glm::detail::tvec2<T>& v)
  {
    is >> v[0] >> v[1];
    return is;
  }

  template<typename T>
  std::istream& operator >> (std::istream& is, glm::detail::tvec3<T>& v)
  {
    is >> v[0] >> v[1] >> v[2];
    return is;
  }

  template<typename T>
  std::istream& operator >> (std::istream& is, glm::detail::tvec4<T>& v)
  {
    is >> v[0] >> v[1] >> v[2] >> v[3];
    return is;
  }

  template<typename T>
  std::istream& operator >> (std::istream& is, glm::detail::tmat2x2<T>& A)
  {
    glm::detail::tvec2<T> r0, r1;
    is >> r0 >> r1;
    A[0][0] = r0.x; A[1][0] = r0.y;
    A[0][1] = r1.x; A[1][1] = r1.y;
    return is;
  }

  template<typename T>
  std::istream& operator >> (std::istream& is, glm::detail::tmat3x3<T>& A)
  {
    glm::detail::tvec3<T> r0, r1, r2;
    is >> r0 >> r1 >> r2;
    A[0][0] = r0.x; A[1][0] = r0.y; A[2][0] = r0.z;
    A[0][1] = r1.x; A[1][1] = r1.y; A[2][1] = r1.z;
    A[0][2] = r2.x; A[1][2] = r2.y; A[2][2] = r2.z;
    return is;
  }

  template<typename T>
  std::istream& operator >> (std::istream& is, glm::detail::tmat4x4<T>& A)
  {
    glm::detail::tvec4<T> r0, r1, r2, r3;
    is >> r0 >> r1 >> r2 >> r3;
    A[0][0] = r0.x; A[1][0] = r0.y; A[2][0] = r0.z; A[3][0] = r0.w;
    A[0][1] = r1.x; A[1][1] = r1.y; A[2][1] = r1.z; A[3][1] = r1.w;
    A[0][2] = r2.x; A[1][2] = r2.y; A[2][2] = r2.z; A[3][2] = r2.w;
    A[0][3] = r3.x; A[1][3] = r3.y; A[2][3] = r3.z; A[3][3] = r3.w;
    return is;
  }


}


#endif  // glmutils_istream_hpp
