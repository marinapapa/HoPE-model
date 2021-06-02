#ifndef GLMUTILS_SMOOTHERSTEP_HPP_INCLUDED
#define GLMUTILS_SMOOTHERSTEP_HPP_INCLUDED

#include <glm/glm.hpp>


namespace glmutils {


  template <typename T>
  inline T smootherstep(T const& e0, T const& e1, T const& x)
  {
    const float t = glm::clamp<T>((x-e0)/(e1-e0), 0.0f, 1.0f);
    return t*t*t*(t*(t*6 - 15) + 10);
  }


}

#endif

