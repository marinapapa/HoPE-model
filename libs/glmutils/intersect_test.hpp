//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup intersection Intersection tests
//! \file intersect_test.hpp Defines the enumeration returned by various intersection tests.
//! \ingroup intersection


#ifndef glmutils_intersect_test_hpp
#define glmutils_intersect_test_hpp


namespace glmutils {


  //! \ingroup intersection
  enum intersect_test {
    OUTSIDE,
    INSIDE,
    INTERSECTING
  };


}   // namespace glmutils


#endif  // glmutils_intersect_test_hpp
