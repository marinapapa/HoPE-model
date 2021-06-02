//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file bbox_ray.hpp Bounding box-ray intersection tests.
//! \ingroup bbox intersection ray
//
// Based on:
// Amy Williams et al.; An Efficient and Robust Ray-Box intersection Algorithm
// JGT Vol. 10 No 1: p55-60
//


#ifndef glmutils_bbox_line_hpp
#define glmutils_bbox_line_hpp


#include <glmutils/bbox.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;


  //! Bounding box-ray intersection test.
  //!
  //! Based on:
  //! Amy Williams et al.; An Efficient and Robust Ray-Box intersection Algorithm
  //! JGT Vol. 10 No 1: p55-60
  //!
  //! \param[in] box The bounding box
  //! \param[in] orig origin of the ray
  //! \param[in] dir direction of the ray
  //! \param[in] t0 smallest ray parameter t, see \ref ray
  //! \param[in] t1 largest ray parameter t, see \ref ray
  //!
  template<typename bbox>
  inline bool intersect(const bbox& box,
              const typename bbox::vec_type& orig,
              const typename bbox::vec_type& dir,
              typename bbox::value_type t0,
              typename bbox::value_type t1)
  {
    typedef typename bbox::value_type T;
    const typename bbox::vec_type inv_dir = T(1) / dir;
    int s = (inv_dir[0] < T(0));
    T tmin = (box[s][0] - orig[0]) * inv_dir[0];
    T tmax = (box[1-s][0] - orig[0]) * inv_dir[0];
    for (int i=1; i<trait<typename bbox::vec_type>::size; ++i)
    {
      int s = (inv_dir[i] < T(0));
      const T tmini = (box[s][i] - orig[i]) * inv_dir[i];
      const T tmaxi = (box[1-s][i] - orig[i]) * inv_dir[i];
      if ( (tmin > tmaxi) || (tmini > tmax) )
      {
        return false;
      }
      if (tmini > tmin) { tmin = tmini; }
      if (tmaxi < tmax) { tmax = tmaxi; }
    }
    return ( (tmin < t1) && (tmax > t0) );
  }


  //! Optimized bounding box-ray intersection test.
  //!
  //! Based on:
  //! Amy Williams et al.; An Efficient and Robust Ray-Box intersection Algorithm
  //! JGT Vol. 10 No 1: p55-60
  //!
  //! \param[in] box The bounding box
  //! \param[in] orig origin of the ray
  //! \param[in] inv_dir inversed direction of the ray
  //! \param[in] sign sign of inv_dir: \f$( inv\_dir.x < 0, inv\_dir.y < 0, ...)\f$
  //! \param[in] t0 smallest ray parameter t, see \ref ray
  //! \param[in] t1 largest ray parameter t, see \ref ray
  //!
  //! \note Optimized for testing many boxes against the same ray since
  //! \a inv_dir and \a sign are constant "per ray".
  //!
  template<typename bbox>
  inline bool intersect(const bbox& box,
              const typename bbox::vec_type& orig,
              const typename bbox::vec_type& inv_dir,
              const typename trait<typename bbox::vec_type>::template other<int>::type& sign,
              typename bbox::value_type t0,
              typename bbox::value_type t1)
  {
    typedef typename bbox::value_type T;
    T tmin = (box[sign[0]][0] - orig[0]) * inv_dir[0];
    T tmax = (box[1-sign[0]][0] - orig[0]) * inv_dir[0];
    for (int i=1; i<trait<typename bbox::vec_type>::size; ++i)
    {
      const T tmini = (box[sign[i]][i] - orig[i]) * inv_dir[i];
      const T tmaxi = (box[1-sign[i]][i] - orig[i]) * inv_dir[i];
      if ( (tmin > tmaxi) || (tmini > tmax) )
      {
        return false;
      }
      if (tmini > tmin) { tmin = tmini; }
      if (tmaxi < tmax) { tmax = tmaxi; }
    }
    return ( (tmin < t1) && (tmax > t0) );
  }



}   // namespace glmutils


#endif  // glmutils_bbox_ray_hpp
