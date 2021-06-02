//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

// \defgroup quad Quads
// Quads are nothing more than 4-gons.
//! \file quad.hpp Point in Quad test.
//! \ingroup quad poly intersection


#ifndef glmutils_quad_hpp
#define glmutils_quad_hpp


#include <limits>
#include <glm/glm.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/ray.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Point in Quad test 2D.
  //! \param[in] p The point to test.
  //! \param[in] v0 1st point of the quad.
  //! \param[in] v1 2nd point of the quad.
  //! \param[in] v2 3rd point of the quad.
  //! \param[in] v3 4th point of the quad.
  template<typename T>
  inline bool pointInQuad(const tvec2<T>& p,
              const tvec2<T>& v0, const tvec2<T>& v1,
              const tvec2<T>& v2, const tvec2<T>& v3)
  {
    bool inside = false;
    bool y0 = (v0[1] >= p[1]);
    bool y1 = (v1[1] >= p[1]);
    if ((y0 != y1) && (((v1[1]-p[1])*(v0[0]-v1[0]) >= (v1[0]-p[0])*(v0[1]-v1[1])) == y1))
    {
      inside = !inside;
    }
    bool y2 = (v2[1] >= p[1]);
    if ((y1 != y2) && (((v2[1]-p[1])*(v1[0]-v2[0]) >= (v2[0]-p[0])*(v1[1]-v2[1])) == y2))
    {
      inside = !inside;
    }
    bool y3 = (v3[1] >= p[1]);
    if ((y2 != y3) && (((v3[1]-p[1])*(v2[0]-v3[0]) >= (v3[0]-p[0])*(v2[1]-v3[1])) == y3))
    {
      inside = !inside;
    }
    if ((y0 != y3) && (((v3[1]-p[1])*(v0[0]-v3[0]) >= (v3[0]-p[0])*(v0[1]-v3[1])) == y3))
    {
      inside = !inside;
    }
    return inside;
  }


  //! Point in Quad test in 2D
  //! \param[in] p The point to test
  //! \param[in] v Random access iterator referencing the 4 vertices of the quad.
  template<typename IT>
  inline bool pointInQuad(const tvec2<typename iter_trait<IT>::elem_type>& p, IT v)
  {
    bool inside = false;
    bool y0 = (v[0][1] >= p[1]);
    bool y1 = (v[1][1] >= p[1]);
    if ((y0 != y1) && (((v[1][1]-p[1])*(v[0][0]-v[1][0]) >= (v[1][0]-p[0])*(v[0][1]-v[1][1])) == y1))
    {
      inside = !inside;
    }
    bool y2 = (v[2][1] >= p[1]);
    if ((y1 != y2) && (((v[2][1]-p[1])*(v[1][0]-v[2][0]) >= (v[2][0]-p[0])*(v[1][1]-v[2][1])) == y2))
    {
      inside = !inside;
    }
    bool y3 = (v[3][1] >= p[1]);
    if ((y2 != y3) && (((v[3][1]-p[1])*(v[2][0]-v[3][0]) >= (v[3][0]-p[0])*(v[2][1]-v[3][1])) == y3))
    {
      inside = !inside;
    }
    if ((y0 != y3) && (((v[3][1]-p[1])*(v[0][0]-v[3][0]) >= (v[3][0]-p[0])*(v[0][1]-v[3][1])) == y3))
    {
      inside = !inside;
    }
    return inside;
  }


}   // namespace glmutils


#endif  // glmutils_quad_hpp
