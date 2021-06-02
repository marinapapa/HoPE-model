//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file oobb.hpp Object oriented bounding box calculation.
//! \ingroup bbox



#ifndef glmutils_oobb_hpp
#define glmutils_oobb_hpp


#include <glmutils/traits.hpp>
#include <glmutils/fast_eigen.hpp>
#include <glmutils/bbox.hpp>

namespace glmutils {

    using namespace glm;
  using namespace glm::detail;

  //! Calculation of a object oriented bounding box by means of PCA.
  //!
  //! \param[in] n number of points.
  //! \param[in] v random access iterator referencing the points.
  //! \param[out] EXT extend of the OOBB
  //! \return Homogeneous transformation matrix oobb -> Euclidean.
  //!
  template<typename IT>
  inline typename trait< typename trait< typename iter_trait<IT>::value_type >::homogeneous>::square
  oobb(const int n, IT v, typename iter_trait<IT>::value_type& EXT)
  {
    typedef typename iter_trait<IT>::elem_type T;
    typedef typename iter_trait<IT>::value_type vec_type;
    typedef tbbox<vec_type> box_type;
    typedef typename trait<vec_type>::square mat_type;
    typedef typename trait< typename trait<vec_type>::homogeneous >::square hmat;
    
    T scale = T(1)/T(n);
    vec_type c(0);
    for (int i=0; i<n; ++i) c += v[i];
    c *= scale;
    mat_type A(0);
    for (int i=0; i<n; ++i)
    {
      const vec_type x(v[i] - c);
      A += outerProduct(x, x);
    }
    A *= scale;
    mat_type EV;
    fast_eig(A, EV);
    box_type oobb(v[0] * EV);
    for (int i=1; i<n; ++i)
    {
      include(oobb, v[i] * EV);
    }
    EXT = extent(oobb);
    hmat H(EV);
    vec_type gc = EV * center(oobb);
    for (int i=0; i<trait<hmat>::rows-1; ++i) H[trait<hmat>::columns-1][i] = gc[i];
    return H;
  }


}   // namespace glmutils


#endif  // glmutils_oobb_hpp
