//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file gram_schmidt.hpp Gram-Schmidt orthogonalization.
//! \ingroup linalg


#ifndef glmutils_gram_schmidt_hpp
#define glmutils_gram_schmidt_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {


  //! Gram-Schmidt orthogonalization
  //! \param[in] A Argument matrix
  //! \return The Orthogonal matrix of the basis vectors in A
  template<typename M>
  inline M gram_schmidt(const M& A)
  {
    using namespace glm;

    M Q(A);
    Q[0] /= length(Q[0]);
    for (int j=1; j<trait<M>::col_size; ++j)
    {
      for (int k=j; k<trait<M>::col_size; ++k)
      {
        Q[k] -= dot( Q[k], Q[j-1] ) * Q[j-1];
      }
      Q[j] /= length(Q[j]);
    }
    return Q;
  }


}  // namespace glmutils


#endif  // glmutils_gram_schmidt_hpp
