//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup homogeneous Homogeneous coordinates
//! \file homogeneous.hpp Support for homogeneous coordinates in 2D and 3D \n
//! \ingroup homogeneous transform


#ifndef glmutils_homogenenous_hpp
#define glmutils_homogenenous_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;

  //! Returns the point \a p in homogeneous coordinates (2D)
  template<typename T>
  inline tvec3<T> point2hom(const tvec2<T>& p) { return tvec3<T>(p, T(1)); }


  //! Returns the point \a p in homogeneous coordinates (3D)
  template<typename T>
  inline tvec4<T> point2hom(const tvec3<T>& p) { return tvec4<T>(p, T(1)); }


  //! Returns the vector \a v in homogeneous coordinates (2D)
  template<typename T>
  inline tvec3<T> vector2hom(const tvec2<T>& v) { return tvec3<T>(v, T(0)); }


  //! Returns the vector \a p in homogeneous coordinates (3D)
  template<typename T>
  inline tvec4<T> vector2hom(const tvec3<T>& v) { return tvec4<T>(v, T(0)); }


  //! Returns the homogeneous point \a p in Cartesian coordinates (2D)
  template<typename T>
  inline tvec2<T> hom2point(const tvec3<T>& h) { return tvec2<T>(h[0] / h[2], h[1] / h[2]); }


  //! Returns the homogeneous point \a p in Cartesian coordinates (3D)
  template<typename T>
  inline tvec3<T> hom2point(const tvec4<T>& h) { return tvec3<T>(h[0] / h[3], h[1] / h[3], h[2] / h[3]); }


  //! Returns the homogeneous vector \a v in Cartesian coordinates (2D)
  template<typename T>
  inline tvec2<T> hom2vector(const tvec3<T>& h) { return tvec2<T>(h[0], h[1]); }


  //! Returns the homogeneous vector \a v in Cartesian coordinates (2D)
  template<typename T>
  inline tvec3<T> hom2vector(const tvec4<T>& h) { return tvec3<T>(h[0], h[1], h[2]); }


  //! Homogeneous transformation of the Cartesian vector \a v (2D)
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] v Cartesian vector
  //!
  template<typename T>
  inline tvec2<T> transformVector(const tmat3x3<T>& M, const tvec2<T>& v)
  {
    return tvec2<T>( M[0][0] * v.x + M[1][0] * v.y,
              M[0][1] * v.x + M[1][1] * v.y );
  }


  //! Homogeneous transformation of the Cartesian vector \a v (3D)
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] v Cartesian vector
  //!
  template<typename T>
  inline tvec3<T> transformVector(const tmat4x4<T>& M, const tvec3<T>& v)
  {
    return tvec3<T>( M[0][0] * v.x + M[1][0] * v.y + M[2][0] * v.z,
              M[0][1] * v.x + M[1][1] * v.y + M[2][1] * v.z,
              M[0][2] * v.x + M[1][2] * v.y + M[2][2] * v.z );
  }


  //! Homogeneous transformation of the Cartesian point \a p (2D)
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] v Cartesian point
  //!
  template<typename T>
  inline tvec2<T> transformPoint(const tmat3x3<T>& M, const tvec2<T>& v)
  {
    const T s = T(1) / (M[0][2] * v.x + M[1][2] * v.y + M[2][2]);
    return tvec2<T>( (M[0][0] * v.x + M[1][0] * v.y + M[2][0]) * s,
              (M[0][1] * v.x + M[1][1] * v.y + M[2][1]) * s );
  }


  //! Homogeneous transformation of the Cartesian point \a p (3D)
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] v Cartesian point
  //!
  template<typename T>
  inline tvec3<T> transformPoint(const tmat4x4<T>& M, const tvec3<T>& v)
  {
    const T s = T(1) / (M[0][3] * v.x + M[1][3] * v.y + M[2][3] * v.z + M[3][3]);
    return tvec3<T>( (M[0][0] * v.x + M[1][0] * v.y + M[2][0] * v.z + M[3][0]) * s,
              (M[0][1] * v.x + M[1][1] * v.y + M[2][1] * v.z + M[3][1]) * s,
              (M[0][2] * v.x + M[1][2] * v.y + M[2][2] * v.z + M[3][2]) * s );
  }


  //! Homogeneous transformation of Cartesian vectors in 2D.
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] n Number of vectors
  //! \param[in] v Forward iterator referencing 2-component vector type
  //! \param[in] out Forward iterator referencing 2-component vector type
  //!
  template<typename IIT, typename OIT>
  inline void transformVectors(const tmat3x3<typename iter_trait<IIT>::elem_type>& M,
                 const int n, IIT v, OIT out)
  {
    for (int i=0; i<n; ++i, ++v, ++out)
    {
      *out = transformVector(M, *v);
    }
  }


  //! Homogeneous transformation of Cartesian vectors in 3D.
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] n Number of vectors
  //! \param[in] v Forward iterator referencing 3-component vector type
  //! \param[in] out Forward iterator referencing 3-component vector type
  //!
  template<typename IIT, typename OIT>
  inline void transformVectors(const tmat4x4<typename iter_trait<IIT>::elem_type>& M,
                 const int n, IIT v, OIT out)
  {
    for (int i=0; i<n; ++i, ++v, ++out)
    {
      *out = transformVector(M, *v);
    }
  }


  //! Homogeneous transformation of Cartesian points in 2D.
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] n Number of points
  //! \param[in] v Forward iterator referencing 2-component vector type
  //! \param[in] out Forward iterator referencing 2-component vector type
  //!
  template<typename IIT, typename OIT>
  inline void transformPoints(const tmat3x3<typename iter_trait<IIT>::elem_type>& M,
                const int n, IIT v, OIT out)
  {
    for (int i=0; i<n; ++i, ++v, ++out)
    {
      *out = transformPoint(M, *v);
    }
  }


  //! Homogeneous transformation of Cartesian points in 3D.
  //! \param[in] M Homogeneous transformation matrix
  //! \param[in] n Number of points
  //! \param[in] v Forward iterator referencing 3-component vector type
  //! \param[in] out Forward iterator referencing 3-component vector type
  //!
  template<typename IIT, typename OIT>
  inline void transformPoints(const tmat4x4<typename iter_trait<IIT>::elem_type>& M,
                const int n, IIT v, OIT out)
  {
    for (int i=0; i<n; ++i, ++v, ++out)
    {
      *out = transformPoint(M, *v);
    }
  }


}   // namespace glmutils


#endif  // glmutils_homogeneous_hpp
