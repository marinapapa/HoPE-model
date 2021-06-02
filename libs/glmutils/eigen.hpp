//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup linalg Linear Algebra
//! \n Eigenvalues and Eigenvectors \n
//! Solving Linear Systems \n
//! Matrix factorization \n
//! etc.
//!
//! \file eigen.hpp Eigenvalues and Eigenvectors.
//! \ingroup linalg


#ifndef glmutils_eigen_hpp
#define glmutils_eigen_hpp


#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glmutils/index_min_max.hpp>
#include <glmutils/gauss.hpp>
#include <glmutils/qr.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {


  namespace detail {

    //! \internal
    template<typename M>
    inline M shift_diag(M const& A, typename trait<M>::value_type s)
    {
      M tmp(A);
      const int n = (trait<M>::columns < trait<M>::rows) ? trait<M>::columns : trait<M>::rows;
      for (int i=0; i<n; ++i)  tmp[i][i] += s;
      return tmp;
    }


    //! \internal
    template<typename M>
    inline M& shift_diag_inplace(M & A, typename trait<M>::value_type s)
    {
      const int n = (trait<M>::columns < trait<M>::rows) ? trait<M>::columns : trait<M>::rows;
      for (int i=0; i<n; ++i)  A[i][i] += s;
      return A;
    }


    //! \internal
    template<typename V>
    inline V abs_rank(V const& x)
    {
      // bubble sort
      V v(x);
      for (int p=0; p<trait<V>::size-1; ++p)
      {
        typename trait<V>::value_type ap = std::abs(v[p]);
        for (int i=p+1; i<trait<V>::size; ++i)
        {
          const typename trait<V>::value_type ai = std::abs(v[i]);
          if (ap > ai)
          {
            std::swap(v[p], v[i]);
            ap = ai;
          }
        }
      }
      return v;
    }


    //! \internal
    template<typename Mat, int N>
    struct eigenvalues
    {
      static void result(Mat const& M, double* pE)
      {
        using namespace glm;
        int k = 0;
        Mat A(M);
        for (; k<1000; ++k)
        {
          const double shift = A[N-1][N-1];
          const Mat R(qr_factor(shift_diag_inplace(A, -shift)));
          A = shift_diag(R*A, +shift);
          if (std::abs(shift - A[N-1][N-1]) < 10e-10)
          {
            typedef typename trait<Mat>::reduced X;
            eigenvalues<X,N-1>::result(X(A),pE);      // Deflate
            break;
          }
          if (k == 1000) 
          {
            throw std::invalid_argument("QR iteration does not converge");
          }
        }
        pE[N-1] = A[N-1][N-1];
      }
    };


    //! \internal
    template<typename Mat>
    struct eigenvalues<Mat,2>
    {
      static void result(glm::tmat2x2<double> const& M, double* pE)
      {
        using namespace glm;
        int k = 0;
        double shift;
        Mat A(M);
        for (; k<1000; ++k)
        {
          shift = A[1][1];
          const Mat R(qr_factor(shift_diag_inplace(A, -shift)));
          A = shift_diag(R*A, +shift);
          if (std::abs(shift - A[1][1]) < 10e-10)
          {
            break;
          }
        }
        if (k == 1000) 
        {
          throw std::invalid_argument("QR iteration does not converge");
        }
        pE[0] = A[0][0];
        pE[1] = A[1][1];
      }
    };

  }


  //! Eigenvalues
  //!
  //! Uses the QR algorithm with simple shift and deflation.
  //!
  //! \param[in] A Argument matrix
  //! \return The eigenvalues in increasing L1-order.
  //! \note Throws std::invalid_argument if the QR-iteration does not converge.
  //!
  template<typename M>
  inline typename trait<M>::col_type eig(const M& A)
  {
    using namespace glm;
    using namespace glmutils::detail;

    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::template other<double>::type dMat;
    typedef typename trait<dMat>::col_type dVec;

    dVec E;
    eigenvalues< dMat,trait<M>::col_size >::result(dMat(A), glm::value_ptr(E));

    typename trait<M>::col_type RES;
    typename trait< dVec >::template other<int>::type r = index_rank(E);
    for (int i=0; i<trait<M>::col_size; ++i)
    {
      RES[i] = T(E[r[i]]);
    }
    return detail::abs_rank(RES);
  }


  //! Eigenvalues and Eigenvectors
  //!
  //! Uses the QR algorithm with simple shift and deflation.
  //!
  //! \param[in] A Square Argument matrix
  //! \param[out] EV Matrix of Eigenvectors in the increasing order of the
  //! Eigenvalues.
  //! \return The eigenvalues in increasing L1-order.
  //! \note Throws std::invalid_argument if the QR-iteration does not converge.
  //!
  template<typename M>
  inline typename trait<M>::col_type eig(M const& A, M& EV)
  {
    using namespace glm;
    using namespace glmutils::detail;

    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type vec;
    typedef typename trait<M>::template other<double>::type dMat;
    typedef typename trait<dMat>::transposed tMat;
    typedef typename trait<dMat>::col_type dVec;

    dMat Q(A);
    dMat Z;
    dVec E = eig(Q);
    zero(EV);
    for (int i=0; i<trait<dMat>::col_size; ++i)
    {
      int rank = null(detail::shift_diag(Q, -E[i]), Z);
      if (trait<M>::columns != rank) EV[i] = vec( normalize(Z[0]) );
    }
    return vec(E);
  }


  //! Returns the biggest error in A*EV[i] = E[i]*EV[i].
  template<typename M>
  inline typename trait<M>::value_type eig_error(const M& A, const M& EV, const typename trait<M>::col_type& E)
  {
    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type V;

    T error(0);
    for (int i=0; i<trait<M>::col_size; ++i)
    {
      const V E0 = A*EV[i] - E[i]*EV[i];
      T e = length2(E0);
      if (e > error)
      {
        error = e;
      }
    }
    return error;
  }


  //! Returns the errors in  A*EV = E*EV.
  template<typename M>
  inline typename trait<M>::value_type eig_error(const M& A, const typename trait<M>::col_type& EV, const typename trait<M>::value_type& E)
  {
    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type V;

    T error(0);
    const V E0 = A*EV - E*EV;
    return length2(E0);
  }


}  // namespace glmutils


#endif  // glmutils_eigen_hpp
