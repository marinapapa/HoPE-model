//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \file polygon_moments.hpp Area, Centroid and Inertia of planar polygons in 2D and 3D; \n
//! The calculations are \b valid for \b concave polygons too (hurray!); \n
//! 3D polygons are expected to be planar.
//!
//! <b>Stokes's Theorem</b>
//! \f[ \int_{area} \left( - \frac{dP}{dy} + \frac{dQ}{dx} \right) \, dx dy = \int_{boundary} \left( P dx + Q dy \right) \f]
//! <b>The zeroth moment</b> (\ref glmutils::polygonArea "area")
//! \f[ A = \int_{area} dx dy = \int_{boundary} x \, dy \qquad ; P = 0, \quad Q = x, \quad dQ/dx = 1 \f]
//! <b>The first moment</b> (\ref glmutils::polygonCentroid "centroid")
//! \f[ C_x = \int_{area} x \, dx dy = \frac{1}{2} \int_{boundary} x^2 \, dy \qquad ; P = 0 ,\quad Q = x^2/2, \quad dQ/dx = x \f]
//! \f[ C_y = \int_{area} y \, dx dy = - \frac{1}{2} \int_{boundary} y^2 \, dx \qquad ; P = y^2/2, \quad dP/dy = y, \quad Q = 0 \f]
//! <b>The second moment</b> (\ref glmutils::polygonInertia "area moment of inertia and polar moment of inertia")
//! \f[ I_{xx} = \int_{area} x^2 \, dx dy = \frac{1}{3} \int_{boundary} x^3 \, dy \qquad ; P = 0, \quad Q = x^3/3, \quad dQ/dx = x^2 \f]
//! \f[ I_{yy} = \int_{area} y^2 \, dx dy = - \frac{1}{3} \int_{boundary} y^3 \,dx \qquad ; P = y^3/3, \quad dP/dy = y^2, \quad Q = 0 \f]
//! \f[ I_{polar} = I_{xx} + I_{yy} \f]
//!
//! \ingroup poly


#ifndef glmutils_polygon_moments_hpp
#define glmutils_polygon_moments_hpp


#include <limits>
#include <glm/glm.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/polygon.hpp>
#include <glmutils/plane.hpp>


namespace glmutils {

    using namespace glm;

  //! \cond FORWARD

  template<typename IT>
  inline typename iter_trait<IT>::elem_type
  polygonArea(const int n, IT v);

  template<typename IT>
  inline typename iter_trait<IT>::elem_type
  polygonArea(const int n, const IT v,
        const tvec3< typename iter_trait<IT>::elem_type >& normal);

  template<typename IT>
  inline typename iter_trait<IT>::value_type
  polygonCentroid(const int n, IT v,
          const tvec4< typename iter_trait<IT>::elem_type >& plane,
          typename iter_trait<IT>::elem_type& A);

  //! \endcond

  //! \cond DETAIL
  namespace detail {


    // Returns the polygon area.
    //
    //! \internal
    template<typename IT>
    inline typename iter_trait<IT>::elem_type
    polygonAreaXY(const int X, const int Y, const int n, IT v)
    {
      typedef typename iter_trait<IT>::elem_type T;

      T A = T(0);
      for (int i=0; i<n-1; ++i)
      {
        A += (v[i+1][X] + v[i][X]) * (v[i+1][Y] - v[i][Y]);
      }
      A += (v[0][X] + v[n-1][X]) * (v[0][Y] - v[n-1][Y]);
      return T(0.5) * A;
    }


    // Returns the polygon centroid.
    //
    //! \internal
    template<typename IT>
    inline typename iter_trait<IT>::value_type
    polygonCentroidXY(const int X, const int Y, const int n, IT v,
              typename iter_trait<IT>::elem_type& A)
    {
      typedef typename iter_trait<IT>::value_type VEC;
      typedef typename iter_trait<IT>::elem_type T;

      A = T(0);
      VEC c(0);
      for (int i=0; i<n-1; ++i)
      {
        // dropping canceling terms of form xi*yi
        const T a = v[i][X] * v[i+1][Y] - v[i+1][X] * v[i][Y];
        A += a;
        c[X] += (v[i][X] + v[i+1][X]) * a;
        c[Y] += (v[i][Y] + v[i+1][Y]) * a;
      }
      const T a = v[n-1][X] * v[0][Y] - v[0][X] * v[n-1][Y];
      A += a;
      c[X] += (v[n-1][X] + v[0][X]) * a;
      c[Y] += (v[n-1][Y] + v[0][Y]) * a;
      A *= T(0.5);
      c[X] /= T(6)*A;
      c[Y] /= T(6)*A;
      return c;
    }


    // Returns (Ixx, Iyy, Ipolar) w respect to the origin
    //
    //! \internal
    template<typename IT>
    inline tvec3<typename iter_trait<IT>::elem_type>
    polygonInertiaXY(const int X, const int Y, const int n, IT v,
             typename iter_trait<IT>::elem_type A)
    {
      typedef typename iter_trait<IT>::elem_type T;

      tvec3<T> I(0);
      for (int i=0; i<n-1; ++i)
      {
        T dx = v[i][X] - v[i+1][X];
        T dy = v[i][Y] - v[i+1][Y];
        I.y -= ( v[i][X]*v[i][X]*v[i][X] + v[i][X]*v[i][X]*v[i+1][X] +
             v[i][X]*v[i+1][X]*v[i+1][X] + v[i+1][X]*v[i+1][X]*v[i+1][X] ) * dy;
        I.x += ( v[i][Y]*v[i][Y]*v[i][Y] + v[i][Y]*v[i][Y]*v[i+1][Y] +
             v[i][Y]*v[i+1][Y]*v[i+1][Y] + v[i+1][Y]*v[i+1][Y]*v[i+1][Y] ) * dx;
      }
      T dx = v[n-1][X] - v[0][X];
      T dy = v[n-1][Y] - v[0][Y];
      I.y -= ( v[n-1][X]*v[n-1][X]*v[n-1][X] + v[n-1][X]*v[n-1][X]*v[0][X] +
           v[n-1][X]*v[0][X]*v[0][X] + v[0][X]*v[0][X]*v[0][X] ) * dy;
      I.x += ( v[n-1][Y]*v[n-1][Y]*v[n-1][Y] + v[n-1][Y]*v[n-1][Y]*v[0][Y] +
           v[n-1][Y]*v[0][Y]*v[0][Y] + v[0][Y]*v[0][Y]*v[0][Y] ) * dx;
      I.z = I.x + I.y;
      return I / (A*T(12));
    }


    // Signed polygon area 2D / reduced 3D.
    //
    //! \internal
    template<typename IT>
    inline typename iter_trait<IT>::elem_type
    polygonArea(const int n, IT v, trait< tvec2< typename iter_trait<IT>::elem_type > >)
    {
      return detail::polygonAreaXY(0,1, n, v);
    }


    // Signed polygon area 3D
    //
    //! \internal
    template<typename IT>
    inline typename iter_trait<IT>::elem_type
    polygonArea(const int n, IT v, trait< tvec3< typename iter_trait<IT>::elem_type > >)
    {
      const typename iter_trait<IT>::value_type normal( normalize(cross(v[1] - v[0], v[n-1] - v[0])) );
      return glmutils::polygonArea(n, v, normal);
    }


    // Centroid and signed area of a polygon in 2D
    //
    //! \internal
    template<typename IT>
    inline typename iter_trait<IT>::value_type
    polygonCentroid(const int n, const IT v,
            typename iter_trait<IT>::elem_type& A,
            trait< tvec2< typename iter_trait<IT>::elem_type > >)
    {
      typedef typename iter_trait<IT>::value_type VEC;
      typedef typename iter_trait<IT>::elem_type T;

      VEC c = polygonCentroidXY(0,1, n, v, A);
      return c;
    }


    // Centroid and signed area of a polygon in 3D
    //
    //! \internal
    template<typename IT>
    inline typename iter_trait<IT>::value_type
    polygonCentroid(const int n, const IT v,
            typename iter_trait<IT>::elem_type& A,
            trait< tvec3< typename iter_trait<IT>::elem_type > >)
    {
      typedef typename iter_trait<IT>::elem_type T;
      return ::glmutils::polygonCentroid(n, v, ::glmutils::implicitPlane(v[0], v[1], v[n-1]), A);
    }


  }  // namespace detail
  //! \endcond


  using namespace glm::detail;


  //! Signed polygon area 2D / 3D.
  //!
  //! Use this function in 2D or if nothing more than the polygon vertices is known.
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \return Signed area
  //!
  //! \note CW winding results in negative values see \ref polygonCCW.
  //!
  template<typename IT>
  inline typename iter_trait<IT>::elem_type
  polygonArea(const int n, IT v)
  {
    return detail::polygonArea(n, v, trait< typename iter_trait<IT>::value_type >());
  }


  //! Signed polygon area 3D.
  //!
  //! Use this function if the normal of the polygons plane is known.
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \param[in] normal The normal of the polygons plane
  //! \return Signed area
  //!
  //! \note CW winding results in negative values see \ref polygonCCW.
  //!
  template<typename IT>
  inline typename iter_trait<IT>::elem_type
  polygonArea(const int n, const IT v,
        const tvec3< typename iter_trait<IT>::elem_type >& normal)
  {
    // reduce to two dimensions:
    const int skipCoor = skipCoordinate(normal);
    const int X = (skipCoor == 0) ? 1 : 0;
    const int Y = (skipCoor == 2) ? 1 : 2;
    return detail::polygonAreaXY(X,Y, n, v) / normal[skipCoor];
  }


  //! Centroid and area of a polygon in 2D / 3D
  //!
  //! Use this function in 2D or if nothing more than the polygon vertices is known
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \param[out] A Signed area of the polygon
  //! \return Coordinates of the centroid of the polygon
  //! \note CW winding results in negative values for A see \ref polygonCCW.
  //!
  template<typename IT>
  inline typename iter_trait<IT>::value_type
  polygonCentroid(const int n, IT v,
          typename iter_trait<IT>::elem_type& A)
  {
    return detail::polygonCentroid(n, v, A, trait< typename iter_trait<IT>::value_type >() );
  }


  //! Centroid of a polygon in 2D / 3D
  //!
  //! Use this function in 2D or if nothing more than the polygon vertices is known
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \return Coordinates of the centroid of the polygon
  //!
  template<typename IT>
  inline typename iter_trait<IT>::value_type
  polygonCentroid(const int n, IT v)
  {
    typename iter_trait<IT>::elem_type A;
    return polygonCentroid(n, v, A);
  }


  //! Centroid of a polygon in 3D, known plane
  //!
  //! Use this function if the polygons plane is known.
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \param[in] plane The plane of the polygon
  //! \param[out] A Signed area of the polygon
  //! \return Coordinates of the centroid of the polygon
  //! \note CW winding results in negative values for A see \ref polygonCCW.
  //!
  template<typename IT>
  inline typename iter_trait<IT>::value_type
  polygonCentroid(const int n, IT v,
          const tvec4< typename iter_trait<IT>::elem_type >& plane,
          typename iter_trait<IT>::elem_type& A)
  {
    typedef typename iter_trait<IT>::elem_type T;

    const int skipCoor = skipCoordinate(plane.swizzle(glm::X, glm::Y, glm::Z));
    const int X = (skipCoor == 0) ? 1 : 0;
    const int Y = (skipCoor == 2) ? 1 : 2;
    tvec3<T> c = detail::polygonCentroidXY(X,Y, n, v, A);
    T shearCorrection = T(1) / abs(plane[skipCoor]);
    A *= shearCorrection;
    c *= shearCorrection;
    c[skipCoor] = (-plane.w - dot(plane.swizzle(glm::X, glm::Y, glm::Z), c)) / plane[skipCoor];  // ray-plane intersection
    return c;
  }


  //! Centroid of a polygon in 3D, known plane
  //!
  //! Use this function if the polygons plane is known.
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \param[in] plane The plane of the polygon
  //! \return Coordinates of the centroid (Cx, Cy) of the polygon
  //!
  template<typename IT>
  inline typename iter_trait<IT>::value_type
  polygonCentroid(const int n, IT v,
          const tvec4< typename iter_trait<IT>::elem_type >& plane)
  {
    typename iter_trait<IT>::elem_type A;
    return polygonCentroid(n, v, plane, A);
  }


  //! Returns (Ixx, Iyy, Ipolar) of a polygon w respect to the origin (2D and 3D).
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \return (Ixx, Iyy, Ipolar) assuming a mass of 1
  //! \note Scale the result with the mass of the polygon if it differ from "1".\n
  //!       All the calculated inertias are negative for CW winding, see \ref polygonCCW.
  //!
  template<typename IT>
  inline tvec3< typename iter_trait<IT>::elem_type >
  polygonInertia(const int n, const IT v)
  {
    return detail::polygonInertiaXY(0, 1, n, v, glmutils::polygonArea(n, v));
  }


  //! Returns (Ixx, Iyy, Ipolar) of a polygon w respect to the origin (2D and 3D).
  //!
  //! Use this function the signed polygon area is known.
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices
  //! \param[in] A Signed area of the polygon
  //! \return (Ixx, Iyy, Ipolar) assuming a mass of 1
  //! \note Scale the result with the mass of the polygon if it differ from "1".\n
  //!       All the calculated inertias are negative for CW winding, see \ref polygonCCW.
  //!
  template<typename IT>
  inline tvec3< typename iter_trait<IT>::elem_type >
  polygonInertia(const int n, const IT v, typename iter_trait<IT>::elem_type A)
  {
    return detail::polygonInertiaXY(0, 1, n, v, A);
  }


  //! Returns the inertia with respect to an parallel axis.
  //!
  //!  \param[in] I Known inertia with respect to some axis
  //!  \param[in] M Mass of the body
  //!  \param[in] dSq Squared distance of the parallel axis
  //!
  //!  \par Parallel Axis THeorem
  //! If we know the inertia of a body with respect to one axis, the
  //!  "Parallel Axis Theorem" told us how to get the inertia with
  //!  respect to a parallel axis (and vice versa):
  //!  \f[ I_p = I + M \cdot d^2 \f]
  //!  \f[ I = I_p - M \cdot d^2 \f]
  //!  where \f$I\f$ is the known inertia, \f$M\f$ is the mass of
  //!  the body, \f$d^2\f$ is the squared
  //!  distance of the parallel axis. \f$I_p\f$ is the inertia
  //!  with respect to the parallel axis.
  //!
  template<typename T>
  inline T parallelAxis(T I, T M, T dSq)
  {
    return I + (M * dSq);
  }


  //! Enforces CCW winding of a polygon (2D or 3D).
  //! This function reverses the order of vertices if CW winding is
  //! detected.
  //!
  //! \param[in] n number of vertices.
  //! \param[in,out] V Random access iterator referencing the vertices of the polygon.
  //!
  //! \par Why Counter Clock Wise is the default
  //! The winding of a polygon is determined by its signed area.
  //! The signed area is positive for CCW winding, negative for CW winding.
  //! Unfortunately, the same applies to the higher moments.
  //! Because of this, CCW is the default winding mode in most
  //! applications.\n
  //!
  template<typename IT>
  inline void polygonCCW(const int n, IT V)
  {
    typedef typename iter_trait<IT>::elem_type T;

    if (polygonArea(n, V) < T(0))
    {
      std::reverse(V, V+n);
    }
  }


}   // namespace glmutils


#endif  // glmutils_polygon_moments_hpp
