//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file gauss.hpp Gaussian Elimination \n
//! Null Space
//! \ingroup linalg


#ifndef glmutils_gauss_hpp
#define glmutils_gauss_hpp


#include <limits>
#include <algorithm>
#include <glm/glm.hpp>
#include <glmutils/row_echelon.hpp>
#include <glmutils/zero.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {


  //! Returns the Null Space \c Z of \c A so that A*Z = 0
  //! \param[in] A m x n matrix
  //! \param[out] Z n x m matrix 
  //! \return Rank of A.
  //!
  template<typename M>
  inline int null(const M& A, typename trait<M>::transposed& Z)
  {
    using namespace glm;
    typedef typename trait<M>::value_type T;

    M R(A);
    rref2_result<M> res = rref2(R);
    zero(Z);
    const int n = trait<M>::columns - res.rank;
    for (int i=0; i<n; ++i)
    {
      Z[i][res.nopivid[i]] = T(1);
      for (int r=0; r<res.rank; ++r)
      {
        Z[i][res.pivid[r]] = -R[res.nopivid[i]][r];
      }
    }
    return res.rank;
  }


  //! Particular solution of the linear equations A*x = b.
  //!
  //! Gaussian elimination with scaled partial pivoting.
  //! \param[in] A The coefficient matrix.
  //! \param[in] b Lhs of the equation
  //! \param[out] x The vector \c x that satisfy the equation A*x = b.
  //! \return \c true if a solution exist. 
  //!
  //! Uses Gaussian Elimination with scaled partial pivoting.
  //!
  template<typename M>
  inline bool gauss(const M& A, 
            const typename trait<M>::col_type& b,
            typename trait<M>::row_type& x)
  {
    using namespace glm;
    typedef typename trait<M>::value_type T;

    M B(A);
    typename trait<M>::col_type y(b);
    int rank = rref(B, y);
    const T eps = T(std::max(trait<M>::rows, trait<M>::columns)) * std::numeric_limits<T>::epsilon();
    for (int i=0; i < trait<M>::columns; ++i)
    {
      x[i] = (std::abs(y[i]) > eps) ? y[i] : T(0);
    }
    // Check for consistence
    for (int i=rank; i < trait<M>::rows; ++i)
    {
      if (std::abs(y[i]) > T(0)) return false;
    }
    return true;
  }


}  // namespace glmutils


#endif  // glmutils_gauss_hpp
