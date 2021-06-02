//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2013
//

//! \defgroup fpclassify floating point classification
//! \file fpclassify.hpp floating point classification for glm::tvec types
//! \ingroup bbox


#ifndef glmutils_fpclassify_hpp
#define glmutils_fpclassify_hpp

#include <glm/glm.hpp>


namespace glmutils {

  
  template <typename valType>
  inline bool all_finite(valType x)
  {
    return ! glm::isinf(x);
  }

  template <typename valType>
  inline bool all_finite(const glm::detail::tvec2<valType>& x)
  {
    return ! (glm::isinf<valType>(x.x) || 
      glm::isinf<valType>(x.y));
  }

  template <typename valType>
  inline bool all_finite(const glm::detail::tvec3<valType>& x)
  {
    return ! (glm::isinf<valType>(x.x) || 
      glm::isinf<valType>(x.y) || 
      glm::isinf<valType>(x.z));
  }

  template <typename valType>
  inline bool all_finite(const glm::detail::tvec4<valType>& x)
  {
    return ! (glm::isinf<valType>(x.x) ||
      glm::isinf<valType>(x.y) ||
      glm::isinf<valType>(x.z) || 
      glm::isinf<valType>(x.w));
  }
  
  template <typename valType>
  inline bool all_finite(const glm::detail::tmat2x2<valType>& x)
  {
    return all_finite(x[0]) && 
           all_finite(x[1]);
  }

  template <typename valType>
  inline bool all_finite(const glm::detail::tmat3x3<valType>& x)
  {
    return all_finite(x[0]) && 
           all_finite(x[1]) &&
           all_finite(x[2]);
  }

  template <typename valType>
  inline bool all_finite(const glm::detail::tmat4x4<valType>& x)
  {
    return all_finite(x[0]) && 
           all_finite(x[1]) &&
           all_finite(x[2]) &&
           all_finite(x[3]);
  }


  template <typename valType>
  inline bool any_nan(valType x)
  {
    return glm::isnan(x);
  }

  template <typename valType>
  inline bool any_nan(const glm::detail::tvec2<valType>& x)
  {
    return glm::isnan<valType>(x.x) || 
      glm::isnan<valType>(x.y);
  }

  template <typename valType>
  inline bool any_nan(const glm::detail::tvec3<valType>& x)
  {
    return glm::isnan<valType>(x.x) || 
      glm::isnan<valType>(x.y) || 
      glm::isnan<valType>(x.z);
  }

  template <typename valType>
  inline bool any_nan(const glm::detail::tvec4<valType>& x)
  {
    return glm::isnan<valType>(x.x) || 
      glm::isnan<valType>(x.y) || 
      glm::isnan<valType>(x.z) || 
      glm::isnan<valType>(x.w);
  }

  template <typename valType>
  inline bool any_nan(const glm::detail::tmat2x2<valType>& x)
  {
    return any_nan(x[0]) && 
           any_nan(x[1]);
  }

  template <typename valType>
  inline bool any_nan(const glm::detail::tmat3x3<valType>& x)
  {
    return any_nan(x[0]) && 
           any_nan(x[1]) &&
           any_nan(x[2]);
  }

  template <typename valType>
  inline bool any_nan(const glm::detail::tmat4x4<valType>& x)
  {
    return any_nan(x[0]) && 
           any_nan(x[1]) &&
           any_nan(x[2]) &&
           any_nan(x[3]);
  }


  template <typename valType>
  inline bool any_inf(valType x)
  {
    return glm::isinf(x);
  }

  template <typename valType>
  inline bool any_inf(const glm::detail::tvec2<valType>& x)
  {
    return glm::isinf<valType>(x.x) || 
           glm::isinf<valType>(x.y);
  }

  template <typename valType>
  inline bool any_inf(const glm::detail::tvec3<valType>& x)
  {
    return glm::isinf<valType>(x.x) || 
           glm::isinf<valType>(x.y) || 
           glm::isinf<valType>(x.z);
  }

  template <typename valType>
  inline bool any_inf(const glm::detail::tvec4<valType>& x)
  {
    return glm::isinf<valType>(x.x) || 
           glm::isinf<valType>(x.y) || 
           glm::isinf<valType>(x.z) || 
           glm::isinf<valType>(x.w);
  }

  template <typename valType>
  inline bool any_inf(const glm::detail::tmat2x2<valType>& x)
  {
    return any_inf(x[0]) && 
           any_inf(x[1]);
  }

  template <typename valType>
  inline bool any_inf(const glm::detail::tmat3x3<valType>& x)
  {
    return any_inf(x[0]) && 
           any_inf(x[1]) &&
           any_inf(x[2]);
  }

  template <typename valType>
  inline bool any_inf(const glm::detail::tmat4x4<valType>& x)
  {
    return any_inf(x[0]) && 
           any_inf(x[1]) &&
           any_inf(x[2]) &&
           any_inf(x[3]);
  }

}

#endif
