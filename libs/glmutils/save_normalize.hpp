//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file save_normalize.hpp Normalization with fallback.
//! \ingroup linalg


#ifndef glmutils_save_normalize_hpp
#define glmutils_save_normalize_hpp

#include <glmutils/traits.hpp>


namespace glmutils {


  //! Returns the normalized vector |\x| or \fallback if length is less than \eps
  template <typename V>
  inline V save_normalize(const V& x, const V& fallback, typename trait<V>::value_type eps = 0.000001)
  {
    typename trait<V>::value_type l2 = glm::dot(x, x);
    return (l2 > eps) ? x / sqrt(l2) : fallback;
  }


}

#endif
