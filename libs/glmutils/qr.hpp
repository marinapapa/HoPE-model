//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file qr.hpp QR decomposition in triangular factors.
//! \ingroup linalg


#ifndef glmutils_qr_hpp
#define glmutils_qr_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {


  //! QR-factorization
  //! \param[in,out] Argument matrix. A is replaced by Q.
  //! \return The higher triangular matrix R, so that A = Q*R.
  template<typename M>
  inline M qr_factor(M& A)
  {
    using namespace glm;
    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type col_type;

    M R(T(0));
    const T r00 = length(A[0]);
    if (std::abs(r00) > 10e-10) A[0] *= T(1)/r00;
    R[0][0] = r00;
    for (int j=1; j<trait<M>::col_size; ++j)
    {
      for (int k=j; k<trait<M>::col_size; ++k)
      {
        const T r = dot( A[k], A[j-1] );
        A[k] -= r * A[j-1];
        R[k][j-1] = r;
      }
      const T rjj = length(A[j]);
      if (std::abs(rjj) > 10e-10) A[j] *= T(1)/rjj;
      R[j][j] = rjj;
    }
    return R;
  }


  //! QR-factorization
  //! \param[in] A Argument matrix.
  //! \param[out] Q
  //! \return The higher triangular matrix R, so that A = Q*R.
  template<typename M>
  inline M qr_factor(const M& A, M& Q)
  {
    using namespace glm;
    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type col_type;

    M R(T(0));
    Q = A;
    const T r00 = length(Q[0]);
    Q[0] *= T(1)/r00;
    R[0][0] = r00;
    for (int j=1; j<trait<M>::col_size; ++j)
    {
      for (int k=j; k<trait<M>::col_size; ++k)
      {
        const T r = dot( Q[k], Q[j-1] );
        Q[k] -= r * Q[j-1];
        R[k][j-1] = r;
      }
      const T rjj = length(Q[j]);
      Q[j] *= T(1)/rjj;
      R[j][j] = rjj;
    }
    return R;
  }


}  // namespace glmutils


#endif  // glmutils_qr_hpp
