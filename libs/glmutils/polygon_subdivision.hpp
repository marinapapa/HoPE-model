//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file polygon_subdivision.hpp Polygon subdivision.
//! \ingroup poly


#ifndef glmutils_polygon_subdivision_hpp
#define glmutils_polygon_subdivision_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/polygon.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  /** 4-point subdivision (interpolating scheme) for polygons (2D and 3D)

    \param[in] w Interpolation factor.
    \param[in] k Number of subdivisions.
    \param[in] n Number of vertices.
    \param[in] v Random access iterator referencing the input vertices.
    \param[out] out Random access iterator referencing the resulting \f$2^k \cdot n\f$ vertices.

    \par Definition of the 4-Point Subdivision
    \f[  p_{2i}^j = p_i^j \f]
    \f[ p_{2i+1}^{j+1} = (1/2 + w)( p_i^j + p_{i+1}^j ) - w( p_{i-1}^j + p_{i+2}^j)  \f]
    where \f$p_i\f$ is the ith vertex, the superscript denotes the subdivision round.
    "Good" values for \f$w\f$ are \f$ 0 \le w \le 1/8 \f$. \f$w=0\f$ results in
    linear interpolation.

    \pre The container behind \c out can hold \f$2^k \cdot n\f$ vertices.
  */
  template<typename IIT, typename OIT>
  inline void polygonSD4P(typename iter_trait<IIT>::elem_type w, int k,
              int n, IIT v, OIT out)
  {
    typedef typename iter_trait<IIT>::value_type V;
    typedef typename iter_trait<IIT>::elem_type T;

    const T w0 = T(0.5) + w;
    int s = 1 << k;
    for (int i=0; i<n; ++i)
    {
      out[s*i] = v[i];
    }
    int N = (1 << k) * n;
    for (; k > 0; --k, s >>= 1, n <<= 1)
    {
      for (int i=0; i<n; ++i)
      {
        const int i0 = s*i;              // i
        const int i1 = ngon_index(N, i0 + s);    // i+1
        const int i2 = ngon_index(N, i0 - s);    // i-1
        const int i3 = ngon_index(N, i0 + 2*s);    // i+2
        out[i0 + (s >> 1)] =  w0 * ( out[i0] + out[i1] ) - w * ( out[i2] + out[i3] );
      }
    }
  }


}   // namespace glmutils


#endif  // glmutils_polygon__subdivision_hpp
