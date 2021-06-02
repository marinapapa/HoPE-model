//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup misc Miscellaneous
//! \file clip_length.hpp Clipping and truncation of vectors
//! \ingroup misc


#ifndef glmutils_clip_length_hpp
#define glmutils_clip_length_hpp


#include <glm/glm.hpp>
#include <limits>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Returns the value \c x clipped to \c minLengt <= x <= \maxLength
  template<typename T>
  inline T clipLength(T x, T minLength, T maxLength)
  {
    return (x < minLength) ? minLength : (x > maxLength) ? maxLength : x;
  }


  //! Returns the vector \c x clipped to \c minLength <= length <= \c maxLength
  template<typename T>
  inline tvec2<T> clipLength(const tvec2<T>& v, T minLength, T maxLength)
  {
    T n = length<T>(v);
    T scale = clipLength<T>(n, minLength, maxLength);
    return tvec2<T>(v * (scale/n));
  }


  //! Returns the vector \c x clipped to \c minLength <= length <= \c maxLength
  template<typename T>
  inline tvec3<T> clipLength(const tvec3<T>& v, T minLength, T maxLength)
  {
    T n = length<T>(v);
    T scale = clipLength<T>(n, minLength, maxLength);
    return tvec3<T>(v * (scale/n));
  }


  //! Returns the vector \c x clipped to \c minLength <= length <= \c maxLength
  template<typename T>
  inline tvec4<T> clipLength(const tvec4<T>& v, T minLength, T maxLength)
  {
    T n = length<T>(v);
    T scale = clipLength<T>(n, minLength, maxLength);
    return tvec4<T>(v * (scale/n));
  }


  //! Clips the value \c x clipped to \c minLengt <= x <= \maxLength
  //! \return clipped value
  template<typename T>
  inline T clipLengthInplace(T& x, T minLength, T maxLength)
  {
    x = (x < minLength) ? minLength : (x > maxLength) ? maxLength : x;
    return x;
  }


  //! Clips the vector \c v clipped to \c minLength <= length(v') <= \c maxLength
  //! \return new length of v
  template<typename T>
  inline T clipLengthInplace(tvec2<T>& v, T minLength, T maxLength)
  {
    T n = length<T>(v);
    T scale = clipLength<T>(n, minLength, maxLength);
    v *= (scale/n);
    return scale;
  }


  //! Clips the vector \c v clipped to \c minLength <= length(v') <= \c maxLength
  //! \return new length of v
  template<typename T>
  inline T clipLengthInplace(tvec3<T>& v, T minLength, T maxLength)
  {
    T n = length<T>(v);
    T scale = clipLength<T>(n, minLength, maxLength);
    v *= (scale/n);
    return scale;
  }


  //! Clips the vector \c v clipped to \c minLength <= length(v') <= \c maxLength
  //! \return new length of v
  template<typename T>
  inline tvec4<T> clipLengthInplace(const tvec4<T>& v, T minLength, T maxLength)
  {
    T n = length<T>(v);
    T scale = clipLength<T>(n, minLength, maxLength);
    v *= (scale/n);
    return scale;
  }


  //! Returns the vector \c x truncated to \c length <= \c maxLength
  template<typename T>
  inline T truncateLength(T x, T maxLength)
  {
    return (x > maxLength) ? maxLength : x;
  }


  //! Returns the vector \c x truncated to length <= \c maxLength
  template<typename T>
  inline tvec2<T> truncateLength(const tvec2<T>& v, T maxLength)
  {
    const T n = length<T>(v);
    const T scale = (n > std::numeric_limits<T>::epsilon()) ? truncateLength<T>(n, maxLength) / n : T(1);
    return tvec2<T>(v * scale);
  }


  //! Returns the vector \c x truncated to length <= \c maxLength
  template<typename T>
  inline tvec3<T> truncateLength(const tvec3<T>& v, T maxLength)
  {
    const T n = length<T>(v);
    const T scale = (n > std::numeric_limits<T>::epsilon()) ? truncateLength<T>(n, maxLength) / n : T(1);
    return tvec3<T>(v * scale);
  }


  //! Returns the vector \c x truncated to length <= \c maxLength
  template<typename T>
  inline tvec4<T> truncateLength(const tvec4<T>& v, T maxLength)
  {
    const T n = length<T>(v);
    const T scale = (n > std::numeric_limits<T>::epsilon()) ? truncateLength<T>(n, maxLength) / n : T(1);
    return tvec4<T>(v * scale);
  }


  //! Truncates the value \c x to \c minLength <= \c x' <= \c maxLength
  //! \returns x'
  template<typename T>
  inline T truncateLengthInplace(T x, T maxLength)
  {
    x = (x > maxLength) ? maxLength : x;
    return x;
  }


  //! Truncates the vector \c v' to \c length(v') <= \c maxLength
  //! \returns length(v')
  template<typename T>
  inline T truncateLengthInplace(tvec2<T>& v, T maxLength)
  {
    const T n = length<T>(v);
    if (n > std::numeric_limits<T>::epsilon())
    {
      const T scale = truncateLength<T>(n, maxLength);
      v *= (scale/n);
      return scale;
    }
    return n;
  }


  //! Truncates the vector \c v' to \c length(v') <= \c maxLength
  //! \returns length(v')
  template<typename T>
  inline T truncateLengthInplace(tvec3<T>& v, T maxLength)
  {
    const T n = length<T>(v);
    if (n > std::numeric_limits<T>::epsilon())
    {
      const T scale = truncateLength<T>(n, maxLength);
      v *= (scale/n);
      return scale;
    }
    return n;
  }


  //! Truncates the vector \c v' to \c length(v') <= \c maxLength
  //! \returns length(v')
  template<typename T>
  inline T truncateLengthInplace(tvec4<T>& v, T maxLength)
  {
    const T n = length<T>(v);
    if (n > std::numeric_limits<T>::epsilon())
    {
      const T scale = truncateLength<T>(n, maxLength);
      v *= (scale/n);
      return scale;
    }
    return n;
  }


}   // namespace glmutils


#endif  // glmutils_clip_length_hpp
