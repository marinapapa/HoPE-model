//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file fast_eigen.hpp Explicit and fast calculation of
//! Eigenvalues & Eigenvectors for symmetric 2x2 and 3x3 matrices
//! \ingroup linalg


#ifndef glmutils_fast_eigen_hpp
#define glmutils_fast_eigen_hpp


#include <cassert>
#include <limits>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glmutils/perp_dot.hpp>
#include <glmutils/matrix_norm.hpp>
#include <glmutils/index_min_max.hpp>
#include <glmutils/eigen.hpp>


namespace glmutils {

  using namespace glm;
  using namespace glm::detail;

  namespace detail {


    template<typename T>
    inline tvec2<T> orth_eigen_complement(tmat2x2<T> const& M, T E)
    {
      const T eps = std::numeric_limits<T>::epsilon();
      const T epsSqr = eps * eps;
      tmat2x2<T> A = shift_diag(M, -E);
      int c; T m2 = m2_norm2(A, c);
      return tvec2<T>( (m2 > epsSqr) ? perpDot(A[c] / std::sqrt(m2)) : tvec2<T>(T(1), T(0)) );
    }


    template<typename T>
    inline tvec3<T> orth_eigen_complement(tmat3x3<T> const& M, T E)
    {
      const T eps = std::numeric_limits<T>::epsilon();
      const T epsSqr = eps * eps;
      // Find a reasonable plane normal
      tmat3x3<T> A = shift_diag(M, -E);
      tmat3x3<T> C( cross(A[0], A[1]), cross(A[0], A[2]), cross(A[1], A[2]) );
      tvec3<T> D( length2(C[0]), length2(C[1]), length2(C[2]) );
      int im = index_max(D);
      return (D[im] > epsSqr) ? C[im] / std::sqrt(D[im]) : tvec3<T>(T(0));
    }


    template <typename T>
    inline tvec2<T> orthogonal_unit_vect(tvec2<T> const& v)
    {
      return perpDot(v);
    }


    template <typename T>
    inline tvec3<T> orthogonal_unit_vect(tvec3<T> const& v)
    {
      const T eps = std::numeric_limits<T>::epsilon();
      tvec3<T> perp(0,0,1);
      if (!(std::abs(v.x) <= std::abs(v.z) * eps) || !(std::abs(v.y) <= std::abs(v.z) * eps))
      {
        // x and y are not both too close to z.
        perp = tvec3<T>(-v.y, v.x, T(0));
      }
      else
      {
        perp = cross(v, perp);
      }
      return normalize(perp);
    }



  }


  //! Returns the Eigenvalues of A in increasing L1-order
  //!
  //! \param[in] A Argument 2x2 matrix
  //! \return The eigenvalues in increasing L1-order.
  //! \pre \c A must be a symmetric
  //!
  template <typename T>
  inline tvec2<T> fast_eig(tmat2x2<T> const& A)
  {
    const T eps = std::numeric_limits<T>::epsilon();
    const T epsSqr = eps * eps;
    const T trn = (A[0][0]+A[1][1]) * T(0.5);
    const T D = determinant(A);
    T r = trn*trn - D;
    const T a = (r > epsSqr) ? std::sqrt(r) : T(0);
    return detail::abs_rank(tvec2<T>(trn - a, trn + a));
  }


  //! Returns the Eigenvalues and Eigenvectors of A in increasing L1-order
  //!
  //! \param[in] A Argument 2x2 matrix
  //! \param[out] EV Matrix of Eigenvectors in the order of their Eigenvalues.
  //! \return The eigenvalues in increasing L1-order.
  //! \pre \c A must be symmetric
  //!
  template<typename T>
  inline tvec2<T> fast_eig(tmat2x2<T> const& A, tmat2x2<T>& EV)
  {
    tvec2<T> E = fast_eig(A);
    EV[1] = detail::orth_eigen_complement(A, E[1]);
    EV[0] = perpDot(EV[1]);
    return E;
  }


  //! Returns the Eigenvalues of A in increasing order
  //!
  //! \param[in] A Argument 3x3 matrix
  //! \return The eigenvalues in increasing L1-order.
  //! \pre \c A must be symmetric
  //!
  //! This algorithm solves the eigenvalue problem for the deviatoric 
  //! problem \f$\mathbf{A}' = \mathbf{A} - \frac{1}{3} \mathbf{I} tr(\mathbf{A})\f$. 
  //! The eigenvalues of this problem are shifted eigenvalues of the original
  //! problem: \f$ \eta_i = \lambda_i - \frac{1}{3} tr(\mathbf{A})\f$. 
  //!
  template<typename T>
  inline tvec3<T> fast_eig(tmat3x3<T> const& M)
  {
    const T m = (M[0][0] + M[1][1] + M[2][2]) / T(3);    // 1/3 tr(A)
    tmat3x3<T> A = detail::shift_diag(M, -m);
    const T q = determinant(A) / T(2);
    const T p = ( dot(A[0],A[0]) + dot(A[1],A[1]) + dot(A[2],A[2]) ) / T(6);
    const T r = p*p*p - q*q;
    const T phi = std::atan2(std::sqrt(r), q) / T(3);
    const T a = std::sqrt(p);
    const T b = std::cos(phi);
    const T c = std::sqrt(T(3)) * std::sin(phi);
    return detail::abs_rank(tvec3<T>( m-a*(b+c), m-a*(b-c), m+2*a*b ));
  }


  //! Returns the Eigenvalues and Eigenvectors of A in increasing order
  //!
  //! \param[in] A Argument 3x3 matrix
  //! \param[out] EV Matrix of Eigenvectors in the order of their Eigenvalues.
  //! \return The eigenvalues in increasing L1-order.
  //! \pre \c A must be a symmetric
  //!
  template<typename T>
  inline tvec3<T> fast_eig(tmat3x3<T> const& A, tmat3x3<T>& EV)
  {
    const T eps = std::numeric_limits<T>::epsilon();
    const T epsSqr = eps * eps;
    const tvec3<T> E = fast_eig(A);
    const T d0 = std::abs(E[2]-E[0]);
    if (d0 < eps) 
    { 
      EV = tmat3x3<T>(T(1)); 
    }
    else
    {
      EV[2] = detail::orth_eigen_complement(A, E[2]);
      const T d1 = std::abs(E[2]-E[1]);
      int i = (d0 <= d1) ? 1 : 0;            // select most distinct other eigenvalue
      EV[i] = (std::abs(E[i]) > eps) 
              ? detail::orth_eigen_complement(A, E[i])
              : detail::orthogonal_unit_vect(EV[2]);
      EV[i ^ 1] = cross(EV[2], EV[i]);
      T cl = length2(EV[i ^ 1]);
      if (cl > epsSqr) EV[i ^ 1] /= std::sqrt(cl);
    }
    return E;
  }


}  // namespace glmutils


#endif  // glmutils_fast_eigen_hpp
