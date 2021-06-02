//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup io IO streaming
//! \file ostream.hpp \a ostream operators for vector and matrix types.
//! \ingroup io
//!


#ifndef glmutils_ostream_hpp
#define glmutils_ostream_hpp


#include <iostream>
#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {

  namespace detail {

    template<typename M>
    inline typename trait<M>::row_type get_row(const M& A, int r)
    {
      typename trait<M>::row_type v;
      for (int i=0; i<trait<M>::columns; ++i) {
        v[i] = A[i][r];
      }
      return v;
    }

  }

  template<typename V>
  inline std::ostream& printVec(std::ostream& os, const V& v)
  {
    for (int i=0; i<trait<V>::size-1; ++i) {
      os << v[i] << ' ';
    }
    os << v[trait<V>::size-1];
    return os;
  }

  template<typename M>
  inline std::ostream& printMat(std::ostream& os, const M& A)
  {
    for (int i=0; i<trait<M>::rows-1; ++i) {
      os << detail::get_row(A, i) << '\n';
    }
    os << detail::get_row(A, trait<M>::rows-1);
    return os;
  }

}  // namespace glmutils


namespace std {

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tvec2<T>& v)
  {
    return glmutils::printVec(os, v);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tvec3<T>& v)
  {
    return glmutils::printVec(os, v);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tvec4<T>& v)
  {
    return glmutils::printVec(os, v);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat2x2<T>& A)
  {
    return glmutils::printMat(os, A);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat3x3<T>& A)
  {
    return glmutils::printMat(os, A);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat4x4<T>& A)
  {
    return glmutils::printMat(os, A);
  }


  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat2x3<T>& A)
  {
    return glmutils::printMat(os, A);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat2x4<T>& A)
  {
    return glmutils::printMat(os, A);
  }


  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat3x2<T>& A)
  {
    return glmutils::printMat(os, A);
  }


  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat3x4<T>& A)
  {
    return glmutils::printMat(os, A);
  }


  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat4x2<T>& A)
  {
    return glmutils::printMat(os, A);
  }


  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const glm::detail::tmat4x3<T>& A)
  {
    return glmutils::printMat(os, A);
  }


}   // namespace std


#endif  // glmutils_ostream_hpp
