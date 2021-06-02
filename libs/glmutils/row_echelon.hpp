//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file row_echelon.hpp Row Echelon Form & Rank.
//! \ingroup linalg


#ifndef glmutils_row_echelon_hpp
#define glmutils_row_echelon_hpp


#include <limits>
#include <glm/glm.hpp>
#include <glmutils/gauss.hpp>    // row_scale
#include <glmutils/traits.hpp>

namespace glmutils {


  //! Return type of \c rref2
  template<typename M>
  struct rref2_result
  {
    typedef typename trait< typename trait<M>::row_type >::template other<int>::type pivid_t;  //!< Type of pivot vector
    rref2_result() : rank(0), pivid(-1), nopivid(-1) {}
    int rank;      //!< Rank of the matrix
    pivid_t pivid;    //!< Vector of pivot ids (pivid_i >= 0 => i is base vector. 
    pivid_t nopivid;  //!< Vector of non-pivot ids (nopivid_i >= 0 => x_i is free variable.
  };


  //! Reduced Row Echelon form.
  //!
  //! \param[in,out] A
  //! \return A \ref rref2_result structure.
  //!
  //! The matrix A is transformed by Gauss-Jordan 
  //! Elimination with scaled partial pivoting.
  //!
  template<typename M>
  inline rref2_result<M> rref2(M& A)
  {
    using namespace glm;
    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type cvec;
    typedef typename trait<M>::row_type rvec;

    const T eps = T(0);
    // row scale A so that max(|a_*j|) = 1
    cvec S(0);
    for (int i = 0; i < trait<M>::rows; ++i)
    {
      for (int j = 0; j < trait<M>::columns; ++j)
      {
        T q = std::abs(A[j][i]);
        if (q > S[i]) S[i] = q; 
      }
      T s = (S[i] > eps) ? T(1) / S[i] : T(0);
      for (int j = 0; j < trait<M>::columns; ++j)
      {
        A[j][i] = A[j][i] * s;
      }
    }
    int pc, pr, pivid, nopivid;
    pc = pr = pivid = nopivid = 0;
    rref2_result<M> res;
    while ((pc < trait<M>::columns) && (pr < trait<M>::rows))
    {
      T m = std::abs(A[pc][pr]);
      int mi = pr;
      // search for maximum column entry below pr
      for (int u=pr+1; u < trait<M>::rows; ++u)
      {
        T q = std::abs(A[pc][u]);
        if (m < q)
        {
          m = q;
          mi = u;
        }
      }
      if (m > eps)
      {
        // swap rows if necessary
        if (mi != pr)
        {
          for (int i=pc; i < trait<M>::columns; ++i)
          {
            std::swap(A[i][mi], A[i][pr]);
          }
        }
        // Divide pivot row by the pivot element
        T q = T(1) / A[pc][pr];
        for (int j=pc; j < trait<M>::columns; ++j)
        {
          A[j][pr] *= q;
        }
        // Subtract multiples of the pivot row from all other rows
        for (int i=0; i < trait<M>::rows; ++i)
        {
          if (i != pr)
          {
            const T f = A[pc][i];
            for (int j=pc; j < trait<M>::columns; ++j)
            {
              A[j][i] -= f * A[j][pr];
            }
          }
        }
        ++res.rank;
        res.pivid[pivid++] = pc;
        ++pr;
        ++pc;
      }
      else
      {
        // The column is negligible
        for (int i=pr; i<trait<M>::rows; ++i)
        {
          A[pc][i] = T(0);
        }
        res.nopivid[nopivid++] = pc;
        ++pc;
      }
    }
    return res;
  }


  //! Reduced Row Echelon form of A|b.
  //!
  //! \param[in,out] A
  //! \param[in,out] b
  //! \return A \ref rref2_result structure.
  //!
  //! The matrix A is transformed by Gauss-Jordan 
  //! Elimination with scaled partial pivoting.
  //!
  template<typename M>
  inline rref2_result<M> rref2(M& A, typename trait<M>::col_type& b)
  {
    using namespace glm;

    typedef typename trait<M>::value_type T;
    typedef typename trait<M>::col_type cvec;
    typedef typename trait<M>::row_type rvec;

    const T eps = T(std::max(trait<M>::rows, trait<M>::columns)) * std::numeric_limits<T>::epsilon();
    // Row-scale the system to max(|a_*j|) = 1
    cvec S;
    for (int i = 0; i < trait<M>::rows; ++i)
    {
      S[i] = std::abs(b[i]);
      for (int j = 0; j < trait<M>::columns; ++j)
      {
        T q = std::abs(A[j][i]);
        if (q > S[i]) S[i] = q; 
      }
      T s = (S[i] > eps) ? T(1) / S[i] : T(0);
      for (int j = 0; j < trait<M>::columns; ++j)
      {
        A[j][i] *= s;
      }
      b[i] *= s;
    }
    int pc, pr, pivid, nopivid;
    pc = pr = pivid = nopivid = 0;
    rref2_result<M> res;
    while ((pc < trait<M>::columns) && (pr < trait<M>::rows))
    {
      T m = std::abs(A[pc][pr]);
      int mi = pr;
      // search for maximum column entry below pr
      for (int u=pr+1; u < trait<M>::rows; ++u)
      {
        T q = std::abs(A[pc][u]);
        if (m < q)
        {
          m = q;
          mi = u;
        }
      }
      // swap rows if necessary
      if (mi != pr)
      {
        for (int i=pc; i < trait<M>::columns; ++i)
        {
          std::swap(A[i][mi], A[i][pr]);
        }
        std::swap(b[mi], b[pr]);
      }
      if (m > eps)
      {
        // Gauss-Jordan Elimination
        T q = T(1) / A[pc][pr];
        A[pc][pr] = T(1);
        for (int j=pc+1; j < trait<M>::columns; ++j)
        {
          A[j][pr] *= q;
        }
        b[pr] *= q;
        for (int i=0; i < trait<M>::rows; ++i)
        {
          if (i==pr) continue;
          const T f = A[pc][i];
          A[pc][i] = T(0);
          for (int j=pc+1; j < trait<M>::columns; ++j)
          {
            A[j][i] -= f * A[j][pr];
          }
          b[i] -= b[pr] * f;
        }
        ++res.rank;
        res.pivid[pivid++] = pc;
        ++pr;
        ++pc;
      } 
      else 
      {
        // The column is negligible
        for (int i=pr; i<trait<M>::rows; ++i)
        {
          A[pc][i] = T(0);
        }
        res.nopivid[nopivid++] = pc;
        ++pc;
      }
    }
    return res;
  }


  //! Reduced Row Echelon form.
  //!
  //! \param[in,out] A
  //! \return The rank of A.
  //!
  //! The matrix A is transformed by Gauss-Jordan 
  //! Elimination with scaled partial pivoting.
  //!
  template<typename M>
  inline int rref(M& A)
  {
    rref2_result<M> res = rref2(A);
    return res.rank;
  }


  //! Reduced Row Echelon Form of A|b.
  //!
  //! \param[in,out] A
  //! \param[in,out] b
  //! \return The Rank of A
  //!
  //! The matrix A|b is transformed by means of Gauss-Jordan 
  //! Elimination with scaled partial pivoting.
  //!
  template<typename M>
  inline int rref(M& A, typename trait<M>::col_type& b)
  {
    rref2_result<M> res = rref2(A, b);
    return res.rank;
  }


  //! Rank of a matrix
  //!
  //! \param[in] A
  //! \return The Rank of A
  //!
  template<typename M>
  inline int rank(const M& A)
  {
    M B(A);
    rref2_result<M> res = rref2(B);
    return res.rank;
  }


}  // namespace glmutils


#endif  // glmutils_row_echolon_hpp
