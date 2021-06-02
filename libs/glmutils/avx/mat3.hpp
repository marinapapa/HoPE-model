#ifndef GLMUTILS_AVX_MAT3_HPP_INCLUDED
#define GLMUTILS_AVX_MAT3_HPP_INCLUDED

#include <cassert>
#include "vec.hpp"


namespace glmutils { namespace avx {

  namespace detail {

    inline void transpose3(__m128 const in[3], __m128 out[3])
    {
      __m128 tmp0 = _mm_movelh_ps(in[0], in[1]);
      __m128 tmp1 = _mm_unpackhi_ps(in[0], in[1]);
      out[0] = _mm_shuffle_ps(tmp0, in[2], _MM_SHUFFLE(3,0,2,0));
      out[1] = _mm_shuffle_ps(tmp0, in[2], _MM_SHUFFLE(3,1,3,1));
      out[2] = _mm_shuffle_ps(tmp1, in[2], _MM_SHUFFLE(3,2,1,0));
    }


    inline __m128 mat3_mul(__m128 const M[3], __m128 v)
    {
      __m128 e0 = _mm_permute_ps(v, _MM_SHUFFLE(0,0,0,0));
      __m128 e1 = _mm_permute_ps(v, _MM_SHUFFLE(1,1,1,1));
      __m128 e2 = _mm_permute_ps(v, _MM_SHUFFLE(2,2,2,2));

      __m128 m0 = _mm_mul_ps(M[0], e0);
      __m128 m1 = _mm_mul_ps(M[1], e1);
      __m128 m2 = _mm_mul_ps(M[2], e2);

      __m128 r0 = _mm_add_ps(m0, m1);
      return _mm_add_ps(r0, m2);
    }


    inline __m128 mat3_mul(__m128 v, __m128 const M[3])
    {
      __m128 m0 = _mm_mul_ps(v, M[0]);
      __m128 m1 = _mm_mul_ps(v, M[1]);
      __m128 m2 = _mm_mul_ps(v, M[2]);

      __m128 u0 = _mm_unpacklo_ps(m0, m1);
      __m128 u1 = _mm_unpackhi_ps(m0, m1);
      __m128 a0 = _mm_add_ps(u0, u1);

      __m128 u2 = _mm_unpacklo_ps(m2, m2);
      __m128 u3 = _mm_unpackhi_ps(m2, m2);
      __m128 a1 = _mm_add_ps(u2, u3);

      __m128 r0 = _mm_movelh_ps(a0, a1);
      __m128 r1 = _mm_movehl_ps(a1, a0);
      return _mm_add_ps(r0, r1);
    }


    inline void mat3_mul(__m128 const A[3], __m128 const B[3], __m128 AB[3])
    {
      __m128 a0 = mat3_mul(A, B[0]);
      __m128 a1 = mat3_mul(A, B[1]);
      __m128 a2 = mat3_mul(A, B[2]);
      AB[0] = a0;
      AB[1] = a1;
      AB[2] = a2;
    }

  }
  
  class mat3
  {
  public:
    typedef glm::mat3 glm_type;
    typedef avx::vec3 col_type;
    typedef glm_type::col_type glm_col_type;

    // constructors
    mat3() {}
    explicit mat3(float e) { this->operator = (e); }
    explicit mat3(glm_type const& x) { this->operator = (x); }
    explicit mat3(const __m128* pc) { this->operator = (pc); }
    explicit mat3(const avx::vec3* pc) { this->operator = (pc); }
    mat3(__m128 const& c0, __m128  const& c1, __m128  const& c2, __m128  const& c3)
    {
      c[0].m128 = c0; c[1].m128 = c1; c[2].m128 = c2;
    }
    mat3(avx::vec3 const& c0, avx::vec3  const& c1, avx::vec3  const& c2, avx::vec3  const& c3)
    {
      c[0] = c0; c[1] = c1; c[2] = c2;
    }


    // mutable operators
    mat3& operator = (float e) 
    {
      __m128 zero = _mm_setzero_ps();
      __m128 me = _mm_set_ss(e);
      c[0].m128 = detail::set_x<glm_col_type>(zero, me);
      c[1].m128 = detail::set_y<glm_col_type>(zero, me);
      c[2].m128 = detail::set_z<glm_col_type>(zero, me);
      return *this;
    }
    mat3& operator = (glm_type const& x) { c[0] = x[0]; c[1] = x[1]; c[2] = x[2]; return *this; }
    mat3& operator = (const __m128* pc) { c[0].m128 = pc[0]; c[1].m128 = pc[1]; c[2].m128 = pc[2]; return *this; }
    mat3& operator = (const avx::vec3* pc) { c[0] = pc[0]; c[1] = pc[1]; c[2] = pc[2]; return *this; }

    mat3& operator += (float x) { c[0] += x; c[1] += x; c[2] += x; return *this; }
    mat3& operator -= (float x) { c[0] -= x; c[1] -= x; c[2] -= x; return *this; }
    mat3& operator *= (float x) { c[0] *= x; c[1] *= x; c[2] *= x; return *this; }
    mat3& operator /= (float x) { c[0] /= x; c[1] /= x; c[2] /= x; return *this; }

    mat3& operator += (mat3 const& M) { c[0] += M[0]; c[1] += M[1]; c[2] += M[2]; return *this; }
    mat3& operator -= (mat3 const& M) { c[0] -= M[0]; c[1] -= M[1]; c[2] -= M[2]; return *this; }
    mat3& operator *= (mat3 const& M) { detail::mat3_mul((const __m128*)&c, (const __m128*)&M.c, (__m128*)&c); }
    vec3& operator *= (vec3 const& v) { detail::mat3_mul((const __m128*)&c, v.m128); }

    mat3& transpose() { detail::transpose3((const __m128*)&c, (__m128*)&c); return *this; }

        
    // access
    col_type operator [] (size_t i) const {	assert(i < 3); return c[i]; }
    col_type& operator [] (size_t i) { 	assert(i < 3); return c[i]; }

    // store
    void store(glm_type& dst) { this->store(glm::value_ptr(dst)); }
    void store(float* dst) { for (int i=0; i<3; ++i) detail::store<glm_col_type>(dst + 3*i, c[i].m128); }

    col_type c[3];
  };


  inline bool operator == (mat3 const& a, mat3 const& b)
  {
    for (int i=0; i<3; ++i) { if (a[i] != b[i]) return false; }
    return true;
  }
  inline bool operator != (mat3 const& a, mat3 const& b)
  {
    for (int i=0; i<3; ++i) { if (a[i] == b[i]) return false; }
    return true;
  }

  inline avx::mat3 operator - (avx::mat3 const& M) { return avx::mat3(M) *= -1.0f; }
  inline avx::mat3 operator + (avx::mat3 const& M, float x) { return avx::mat3(M) += x; }
  inline avx::mat3 operator - (float x, avx::mat3 const& M) { return avx::mat3(M) -= x; }
  inline avx::mat3 operator * (avx::mat3 const& M, float x) { return avx::mat3(M) *= x; }
  inline avx::mat3 operator * (float x, avx::mat3 const& M) { return avx::mat3(M) *= x; }
  inline avx::mat3 operator / (avx::mat3 const& M, float x) { return avx::mat3(M) /= x; }

  inline avx::mat3 operator * (avx::mat3 const& A, avx::mat3 const& B)
  {
    avx::mat3 AB;
    detail::mat3_mul((const __m128*)&A, (const __m128*)&B, (__m128*)&AB);
    return AB;
  }
  inline avx::vec3 operator * (avx::mat3 const& M, avx::vec3 const& v) { return avx::vec3(detail::mat3_mul((const __m128*)&M, v.m128)); }
  inline avx::vec3 operator * (avx::vec3 const& v, avx::mat3 const& M) { return avx::vec3(detail::mat3_mul(v.m128, (const __m128*)&M)); }
  

  inline glm::mat3 cast(avx::mat3 const& x)
  {
    return glm::mat3(cast<glm::vec3>(x[0]), cast<glm::vec3>(x[1]), cast<glm::vec3>(x[2]));
  }


  inline avx::mat3 transpose(avx::mat3 const& M)
  {
    return avx::mat3(M).transpose();
  }

}}


#endif
