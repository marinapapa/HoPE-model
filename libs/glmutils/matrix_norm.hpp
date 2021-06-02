//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file matrix_norm.hpp Diverse Matrix Norms
//! \ingroup linalg


#ifndef glmutils_matrix_norm_hpp
#define glmutils_matrix_norm_hpp


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {


  //! Returns the max absolute coefficient
  //!
  template<typename M>
  inline typename trait<M>::value_type maxAbsCoeff(const M& A)
  {
    const typename  trait<M>::value_type* P = glm::value_ptr(A);
    typename trait<M>::value_type res(0);
    for (int i=0; i<trait<M>::size; ++i, ++P)
    {
      typename trait<M>::value_type absCoeff = *P;
      if (absCoeff > res) res = absCoeff;
    }
    return res;
  }


  //! Returns Frobenius Matrix Norm
  //!
  template<typename M>
  inline typename trait<M>::value_type frobenius_norm(const M& A)
  {
    const typename  trait<M>::value_type* P = glm::value_ptr(A);
    typename trait<M>::value_type res(0);
    for (int i=0; i<trait<M>::size; ++i, ++P)
    {
      res += (*P) * (*P);
    }
    return std::sqrt(res);
  }


  //! Returns Frobenius distance
  //!
  template<typename M>
  inline typename trait<M>::value_type frobenius_distance(const M& A, const M& B)
  {
    const typename  trait<M>::value_type* P0 = glm::value_ptr(A);
    const typename  trait<M>::value_type* P1 = glm::value_ptr(B);
    typename trait<M>::value_type res(0);
    for (int i=0; i<trait<M>::size; ++i, ++P0, ++P1)
    {
      typename trait<M>::value_type d = *(P0) - *(P1);
      res += d * d;
    }
    return std::sqrt(res);
  }


  //! Returns squared Frobenius distance
  //!
  template<typename M>
  inline typename trait<M>::value_type frobenius_distance2(const M& A, const M& B)
  {
    const typename  trait<M>::value_type* P0 = glm::value_ptr(A);
    const typename  trait<M>::value_type* P1 = glm::value_ptr(B);
    typename trait<M>::value_type res(0);
    for (int i=0; i<trait<M>::size; ++i, ++P0, ++P1)
    {
      typename trait<M>::value_type d = *(P0) - *(P1);
      res += d * d;
    }
    return res;
  }


  //! Returns the Matrix-1 Norm
  //! \return The largest absolute column sum.
  //!
  template<typename M>
  inline typename trait<M>::value_type m1_norm(const M& A)
  {
    typename trait<M>::value_type res(0);
    for (int j=0; j<trait<M>::columns; ++j)
    {
      typename trait<M>::value_type acs(0);
      for (int i=0; i<trait<M>::rows; ++i)
      {
        acs += std::abs(A[j][i]);
      }
      if (acs > res) { res = acs; }
    }
    return res;
  }



  //! Returns the Matrix-1 Norm
  //! \return The largest absolute column sum and its index.
  //!
  template<typename M>
  inline typename trait<M>::value_type m1_norm(const M& A, int& col)
  {
    typename trait<M>::value_type res(-1);
    for (int j=0; j<trait<M>::columns; ++j)
    {
      typename trait<M>::value_type acs(0);
      for (int i=0; i<trait<M>::rows; ++i)
      {
        acs += std::abs(A[j][i]);
      }
      if (acs > res) { res = acs; col = j; }
    }
    return res;
  }


  //! Returns the Matrix-2 Norm
  //! \return The largest column length.
  //!
  template<typename M>
  inline typename trait<M>::value_type m2_norm(const M& A)
  {
    typename trait<M>::value_type res = length2(A[0]);
    for (int i=1; i<trait<M>::rows; ++i)
    {
      typename trait<M>::value_type len = length2(A[i]);
      if (len > res) { res = len; }
    }
    return std::sqrt(res);
  }


  //! Returns the Matrix-2 Norm
  //! \return The largest absolute column length and its index.
  //!
  template<typename M>
  inline typename trait<M>::value_type m2_norm(const M& A, int& col)
  {
    col = 0;
    typename trait<M>::value_type res = length2(A[0]);
    for (int i=1; i<trait<M>::rows; ++i)
    {
      typename trait<M>::value_type len = length2(A[i]);
      if (len > res) { res = len; col = i; }
    }
    return std::sqrt(res);
  }


  //! Returns the squared Matrix-2 Norm
  //! \return The largest column squared length length.
  //!
  template<typename M>
  inline typename trait<M>::value_type m2_norm2(const M& A)
  {
    typename trait<M>::value_type res = length2(A[0]);
    for (int i=1; i<trait<M>::rows; ++i)
    {
      typename trait<M>::value_type len = length2(A[i]);
      if (len > res) { res = len; }
    }
    return res;
  }


  //! Returns the squared Matrix-2 Norm
  //! \return The largest absolute column squared length and its index.
  //!
  template<typename M>
  inline typename trait<M>::value_type m2_norm2(const M& A, int& col)
  {
    col = 0;
    typename trait<M>::value_type res = length2(A[0]);
    for (int i=1; i<trait<M>::rows; ++i)
    {
      typename trait<M>::value_type len = length2(A[i]);
      if (len > res) { res = len; col = i; }
    }
    return res;
  }


  //! Returns the Matrix-infinity Norm
  //! \return The largest absolute row sum.
  //!
  template<typename M>
  inline typename trait<M>::value_type minf_norm(const M& A)
  {
    typename trait<M>::value_type res(0);
    for (int i=0; i<trait<M>::rows; ++i)
    {
      typename trait<M>::value_type ars(0);
      for (int j=0; j<trait<M>::columns; ++j)
      {
        ars += std::abs(A[j][i]);
      }
      if (ars > res) { res = ars; }
    }
    return res;
  }


  //! Returns the Matrix-infinity Norm
  //! \return The largest absolute row sum and its index..
  //!
  template<typename M>
  inline typename trait<M>::value_type minf_norm(const M& A, int row)
  {
    typename trait<M>::value_type res(-1);
    for (int i=0; i<trait<M>::rows; ++i)
    {
      typename trait<M>::value_type ars(0);
      for (int j=0; j<trait<M>::columns; ++j)
      {
        ars += std::abs(A[j][i]);
      }
      if (ars > res) { res = ars; row = i; }
    }
    return res;
  }


}  // namespace glmutils


#endif  // glmutils_matrix_norm_hpp
