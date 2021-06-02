#ifndef GLMUTILS_AVX_MAT4_HPP_INCLUDED
#define GLMUTILS_AVX_MAT4_HPP_INCLUDED

#include <cassert>
#include "vec.hpp"


namespace glmutils { namespace avx {

  namespace detail {

    inline void transpose4(__m128 const in[4], __m128 out[4])
    {
      __m128 tmp0 = _mm_movelh_ps(in[0], in[1]);
      __m128 tmp1 = _mm_movelh_ps(in[2], in[3]);
      __m128 tmp2 = _mm_movehl_ps(in[1], in[0]);
      __m128 tmp3 = _mm_movehl_ps(in[3], in[2]);

      out[0] = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(2,0,2,0));
      out[1] = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(3,1,3,1));
      out[2] = _mm_shuffle_ps(tmp2, tmp3, _MM_SHUFFLE(2,0,2,0));
      out[3] = _mm_shuffle_ps(tmp2, tmp3, _MM_SHUFFLE(3,1,3,1));
    }


    inline __m128 mat4_mul(__m128 const M[4], __m128 v)
    {
      __m128 e0 = _mm_permute_ps(v, _MM_SHUFFLE(0,0,0,0));
      __m128 e1 = _mm_permute_ps(v, _MM_SHUFFLE(1,1,1,1));
      __m128 e2 = _mm_permute_ps(v, _MM_SHUFFLE(2,2,2,2));
      __m128 e3 = _mm_permute_ps(v, _MM_SHUFFLE(3,3,3,3));

      __m128 m0 = _mm_mul_ps(M[0], e0);
      __m128 m1 = _mm_mul_ps(M[1], e1);
      __m128 m2 = _mm_mul_ps(M[2], e2);
      __m128 m3 = _mm_mul_ps(M[3], e3);

      __m128 r0 = _mm_add_ps(m0, m1);
      __m128 r1 = _mm_add_ps(m2, m3);
      return _mm_add_ps(r0, r1);
    }


    inline __m128 mat4_mul(__m128 v, __m128 const M[4])
    {
      __m128 m0 = _mm_mul_ps(v, M[0]);
      __m128 m1 = _mm_mul_ps(v, M[1]);
      __m128 m2 = _mm_mul_ps(v, M[2]);
      __m128 m3 = _mm_mul_ps(v, M[3]);

      __m128 u0 = _mm_unpacklo_ps(m0, m1);
      __m128 u1 = _mm_unpackhi_ps(m0, m1);
      __m128 a0 = _mm_add_ps(u0, u1);

      __m128 u2 = _mm_unpacklo_ps(m2, m3);
      __m128 u3 = _mm_unpackhi_ps(m2, m3);
      __m128 a1 = _mm_add_ps(u2, u3);

      __m128 r0 = _mm_movelh_ps(a0, a1);
      __m128 r1 = _mm_movehl_ps(a1, a0);
      return _mm_add_ps(r0, r1);
    }


    inline void mat4_mul(__m128 const A[4], __m128 const B[4], __m128 AB[4])
    {
      __m128 a0 = mat4_mul(A, B[0]);
      __m128 a1 = mat4_mul(A, B[1]);
      __m128 a2 = mat4_mul(A, B[2]);
      __m128 a3 = mat4_mul(A, B[3]);
      AB[0] = a0;
      AB[1] = a1;
      AB[2] = a2;
      AB[3] = a3;
    }

  }
  
  class mat4
  {
  public:
    typedef glm::mat4 glm_type;
    typedef avx::vec4 col_type;
    typedef glm_type::col_type glm_col_type;

    // constructors
    mat4() {}
    explicit mat4(float e) { this->operator = (e); }
    explicit mat4(glm_type const& x) { this->operator = (x); }
    explicit mat4(const __m128* pc) { this->operator = (pc); }
    explicit mat4(const avx::vec4* pc) { this->operator = (pc); }
    mat4(__m128 const& c0, __m128  const& c1, __m128  const& c2, __m128  const& c3)
    {
      c[0].m128 = c0; c[1].m128 = c1; c[2].m128 = c2; c[3].m128 = c3;
    }
    mat4(avx::vec4 const& c0, avx::vec4  const& c1, avx::vec4  const& c2, avx::vec4  const& c3)
    {
      c[0] = c0; c[1] = c1; c[2] = c2; c[3] = c3;
    }


    // mutable operators
    mat4& operator = (float e) 
    {
      __m128 zero = _mm_setzero_ps();
      __m128 me = _mm_set_ss(e);
      c[0].m128 = detail::set_x<glm_col_type>(zero, me);
      c[1].m128 = detail::set_y<glm_col_type>(zero, me);
      c[2].m128 = detail::set_z<glm_col_type>(zero, me);
      c[3].m128 = detail::set_w<glm_col_type>(zero, me);
      return *this;
    }
    mat4& operator = (glm_type const& x) { c[0] = x[0]; c[1] = x[1]; c[2] = x[2]; c[3] = x[3]; return *this; }
    mat4& operator = (const __m128* pc) { c[0].m128 = pc[0]; c[1].m128 = pc[1]; c[2].m128 = pc[2]; c[3].m128 = pc[3]; return *this; }
    mat4& operator = (const avx::vec4* pc) { c[0] = pc[0]; c[1] = pc[1]; c[2] = pc[2];  c[3] = pc[3]; return *this; }

    mat4& operator += (float x) { c[0] += x; c[1] += x; c[2] += x; c[3] += x; return *this; }
    mat4& operator -= (float x) { c[0] -= x; c[1] -= x; c[2] -= x; c[3] -= x; return *this; }
    mat4& operator *= (float x) { c[0] *= x; c[1] *= x; c[2] *= x; c[3] *= x; return *this; }
    mat4& operator /= (float x) { c[0] /= x; c[1] /= x; c[2] /= x; c[3] /= x; return *this; }

    mat4& operator += (mat4 const& M) { c[0] += M[0]; c[1] += M[1]; c[2] += M[2]; c[3] += M[3]; return *this; }
    mat4& operator -= (mat4 const& M) { c[0] -= M[0]; c[1] -= M[1]; c[2] -= M[2]; c[3] -= M[3]; return *this; }
    mat4& operator *= (mat4 const& M) { detail::mat4_mul((const __m128*)&c, (const __m128*)&M.c, (__m128*)&c); }
    vec4& operator *= (vec4 const& v) { detail::mat4_mul((const __m128*)&c, v.m128); }

    mat4& transpose() { detail::transpose4((const __m128*)&c, (__m128*)&c); return *this; }

        
    // access
    col_type operator [] (size_t i) const {	assert(i < 4); return c[i]; }
    col_type& operator [] (size_t i) { 	assert(i < 4); return c[i]; }

    // store
    void store(glm_type& dst) { this->store(glm::value_ptr(dst)); }
    void store(float* dst) { for (int i=0; i<4; ++i) detail::store<glm_col_type>(dst + 4*i, c[i].m128); }

    col_type c[4];
  };


  inline bool operator == (mat4 const& a, mat4 const& b)
  {
    for (int i=0; i<4; ++i) { if (a[i] != b[i]) return false; }
    return true;
  }
  inline bool operator != (mat4 const& a, mat4 const& b)
  {
    for (int i=0; i<4; ++i) { if (a[i] == b[i]) return false; }
    return true;
  }

  inline avx::mat4 operator - (avx::mat4 const& M) { return avx::mat4(M) *= -1.0f; }
  inline avx::mat4 operator + (avx::mat4 const& M, float x) { return avx::mat4(M) += x; }
  inline avx::mat4 operator - (float x, avx::mat4 const& M) { return avx::mat4(M) -= x; }
  inline avx::mat4 operator * (avx::mat4 const& M, float x) { return avx::mat4(M) *= x; }
  inline avx::mat4 operator * (float x, avx::mat4 const& M) { return avx::mat4(M) *= x; }
  inline avx::mat4 operator / (avx::mat4 const& M, float x) { return avx::mat4(M) /= x; }

  inline avx::mat4 operator * (avx::mat4 const& A, avx::mat4 const& B)
  {
    avx::mat4 AB;
    detail::mat4_mul((const __m128*)&A, (const __m128*)&B, (__m128*)&AB);
    return AB;
  }
  inline avx::vec4 operator * (avx::mat4 const& M, avx::vec4 const& v) { return avx::vec4(detail::mat4_mul((const __m128*)&M, v.m128)); }
  inline avx::vec4 operator * (avx::vec4 const& v, avx::mat4 const& M) { return avx::vec4(detail::mat4_mul(v.m128, (const __m128*)&M)); }
  

  inline glm::mat4 cast(avx::mat4 const& x)
  {
    return glm::mat4(cast<glm::vec4>(x[0]), cast<glm::vec4>(x[1]), cast<glm::vec4>(x[2]), cast<glm::vec4>(x[3]));
  }


  inline avx::mat4 transpose(avx::mat4 const& M)
  {
    return avx::mat4(M).transpose();
  }

  inline avx::mat4 make_homogenious(glm::mat3 const& R, glm::vec3 const& d)
  {
    avx::mat4 H;
    for (int i=0; i<3; ++i) H[i] = avx::vec4(R[i], 0.0f);
    H[3] = avx::vec4(d, 1.0f);
    return H;
  }

}}


#endif
