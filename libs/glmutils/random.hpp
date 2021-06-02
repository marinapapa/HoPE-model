//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup random Random vectors
//! \file random.hpp Functions to create random vectors
//! using generic random number generators.
//! \ingroup random
//!
//! Intended for use with random number generators from the
//! <A HREF="http://www.boost.org"> boost::random </A> library
//! or the forthcomming standard C++ TR1 \a \<random\> library extension.


#ifndef glmutils_random_hpp
#define glmutils_random_hpp


#include <random>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace glmutils {

  using namespace glm;
  using namespace glm::detail;


  //! Returns a 2D unit vector
  template<typename T, typename ENG>
  inline tvec2<T> unit_vector2(ENG& eng)
  {
    std::uniform_real_distribution<T> rnd(T(-1), T(1));
    return normalize( tvec2<T>(rnd(eng), rnd(eng)) );
  }

  //! Returns a 3D unit vector
  template<typename T, typename ENG>
  inline tvec3<T> unit_vector3(ENG& eng)
  {
    std::uniform_real_distribution<T> rnd(T(-1), T(1));
    return normalize( tvec3<T>(rnd(eng), rnd(eng), rnd(eng)) );
  }

  //! Returns a 4D unit vector
  template<typename T, typename ENG>
  inline tvec4<T> unit_vector4(ENG& eng)
  {
    std::uniform_real_distribution<T> rnd(T(-1), T(1));
    return normalize( tvec4<T>(rnd(eng), rnd(eng), rnd(eng), rnd(eng)) );
  }


  //! Returns a 2D vector in circle
  template<typename T, typename ENG>
  inline tvec2<T> vector2_in_sphere(ENG& eng)
  {
    std::uniform_real_distribution<T> rnd(T(-1), T(1));
    tvec2<T> v;
    do {
      v = tvec2<T>(rnd(eng), rnd(eng));
    } while (T(1) < length2(v));
    return v;
  }

  //! Returns a 3D vector in sphere
  template<typename T, typename ENG>
  inline tvec3<T> vector3_in_sphere(ENG& eng)
  {
    std::uniform_real_distribution<T> rnd(T(-1), T(1));
    tvec3<T> v;
    do
    {
      v = tvec3<T>(rnd(eng), rnd(eng), rnd(eng));
    } while (T(1) < length2(v));
    return v;
  }

  //! Returns a 4D vector in 4D-sphere
  template<typename T, typename ENG>
  inline tvec4<T> vector4_in_sphere(ENG& eng)
  {
    std::uniform_real_distribution<T> rnd(T(-1), T(1));
    tvec4<T> v;
    do
    {
      v = tvec4<T>(rnd(eng), rnd(eng), rnd(eng), rnd(eng));
    } while (T(1) < length2(v));
    return v;
  }


  template<typename ENG> inline vec2 unit_vec2(ENG& eng) { return unit_vector2<float>(eng); }    //!< Calls \ref unit_vector2\<float\>()
  template<typename ENG> inline vec3 unit_vec3(ENG& eng) { return unit_vector3<float>(eng); }    //!< Calls \ref unit_vector3\<float\>()
  template<typename ENG> inline vec4 unit_vec4(ENG& eng) { return unit_vector4<float>(eng); }    //!< Calls \ref unit_vector4\<float\>()

  template<typename ENG> inline vec2 vec2_in_sphere(ENG& eng) { return vector2_in_sphere<float>(eng); }    //!< Calls \ref vector2_in_sphere\<float\>()
  template<typename ENG> inline vec3 vec3_in_sphere(ENG& eng) { return vector3_in_sphere<float>(eng); }    //!< Calls \ref vector3_in_sphere\<float\>()
  template<typename ENG> inline vec4 vec4_in_sphere(ENG& eng) { return vector4_in_sphere<float>(eng); }    //!< Calls \ref vector4_in_sphere\<float\>()


  template<typename ENG> inline dvec2 unit_dvec2(ENG& eng) { return unit_vector2<double>(eng); }    //!< Calls \ref unit_vector2\<double\>()
  template<typename ENG> inline dvec3 unit_dvec3(ENG& eng) { return unit_vector3<double>(eng); }    //!< Calls \ref unit_vector3\<double\>()
  template<typename ENG> inline dvec4 unit_dvec4(ENG& eng) { return unit_vector4<double>(eng); }    //!< Calls \ref unit_vector4\<double\>()

  template<typename ENG> inline dvec2 dvec2_in_sphere(ENG& eng) { return vector2_in_sphere<double>(eng); }    //!< Calls \ref vector2_in_sphere\<double\>()
  template<typename ENG> inline dvec3 dvec3_in_sphere(ENG& eng) { return vector3_in_sphere<double>(eng); }    //!< Calls \ref vector3_in_sphere\<double\>()
  template<typename ENG> inline dvec4 dvec4_in_sphere(ENG& eng) { return vector4_in_sphere<double>(eng); }    //!< Calls \ref vector4_in_sphere\<double\>()


}   // namespace glmutils


#endif  // glmutils_random_hpp
