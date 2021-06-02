//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file zero.hpp Inplace X=0
//! \ingroup linalg


#ifndef glmutils_zero_hpp
#define glmutils_zero_hpp


#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {


  //! Zeros out elements in \c X
  template<typename X>
  inline void zero(X& x)
  {
    typename trait<X>::value_type* p = (typename trait<X>::value_type*)&x[0];
    for (int i=0; i < trait<X>::size; ++i, ++p)
    {
      *p = typename trait<X>::value_type(0);
    }
  }


}  // namespace glmutils


#endif  // glmutils_zero_hpp
