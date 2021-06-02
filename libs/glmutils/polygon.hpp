//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

/** \defgroup poly Polygons

  Polygons are represented as a list of n vertices \b not including
  the closing vertex, most likely stored in a \c std::vector,
  boost::array or plain C-array.


  \file polygon.hpp Calculation of edges and normals from vertices;\n
  Point in polygon tests in 2D and 3D; \n
  3D polygons are expected to be planar.

  \ingroup poly intersection
*/


#ifndef glmutils_polygon_hpp
#define glmutils_polygon_hpp


#include <algorithm>
#include <glm/glm.hpp>
#include <glmutils/traits.hpp>
#include <glmutils/index_min_max.hpp>
#include <glmutils/perp_dot.hpp>


namespace glmutils {

    using namespace glm;
  using namespace glm::detail;

  //! \cond DETAIL
  namespace detail {

    //! \internal
    template<typename IIT, typename OIT>
    inline void polygonNormals(const int n, IIT V, OIT N,
                   trait<tvec2< typename iter_trait<IIT>::elem_type > >)
    {
      for (int i=0; i<n-1; ++i)
      {
        N[i] = normalize( perpDot(V[i]-V[i+1]) );
      }
      N[n-1] = normalize( perpDot(V[n-1]-V[0]) );
    }


    //! \internal
    template<typename IIT, typename OIT>
    inline void polygonNormals(const int n, IIT V, OIT N,
                             trait<tvec3<  typename iter_trait<IIT>::elem_type > >)
    {
      typedef typename iter_trait<IIT>::elem_type T;

      const tvec3<T> planeNormal(cross(V[1] - V[0], V[n-1] - V[0]));
      for (int i=0; i<n-1; ++i)
      {
        N[i] = normalize( cross( planeNormal, V[i]-V[i+1]) );
      }
      N[n-1] = normalize( cross( planeNormal, V[n-1]-V[0]) );
    }


  }  // namespace detail
  //! \endcond


  //! Calculates the circular vertex index of a n-gon.
  //!
  //! Especially useful for bridging the annoying gap between the last and the first vertex.
  //!
  inline int ngon_index(const int n, const int i)
  {
    const int m = i % n;
    return (m >= 0 ? m : n + m);
  }


  //! Calculates the coordinate to skip for plane with normal \c normal
  //! \return The index of the coordinate to skip.
  //! \sa reducePolygon3D2D
  //!
  template<typename T>
  inline int skipCoordinate(const tvec3<T>& normal)
  {
    return index_max(abs(normal));
  }


  //! Calculates the coordinate to skip for a 3D polygon
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Random access iterator referencing to the vertices.
  //! \return The index of the coordinate to skip.
  //! \sa reducePolygon3D2D
  //!
  template<typename IT>
  inline int skipCoordinate(const int n, IT v)
  {
    return skipCoordinate( cross(v[1] - v[0], v[n-1] - v[0]) );
  }


  //! Reduce a 3D polygon to two dimensions.
  //!
  //! This function discards the coordinate for which the corresponding
  //! absolute component of the normal of the polygon is maximal. This way
  //! the projection preserves as much area as possible.
  //!
  //! \param[in] n Number of vertices.
  //! \param[in] v Forward iterator referencing to 3D vertices.
  //! \param[in] out Forward iterator referencing to 2D vertices.
  //! \return The index of the skiped coordinate.
  //
  template<typename IIT, typename OIT>
  inline int reducePolygon3D2D(const int n, IIT v, OIT out)
  {
    const int skipCoor = skipCoordinate(n, v);
    const int X = (skipCoor == 0) ? 1 : 0;
    const int Y = (skipCoor == 2) ? 1 : 2;
    for (int i=0; i<n; ++i, ++v, ++out)
    {
      (*out)[0] = (*v)[X];
      (*out)[1] = (*v)[Y];
    }
    return skipCoor;
  }


  //! Reduce a point in 3D to two dimensions (discards the \c skipCoor).
  template<typename T>
  inline tvec2<T> reducePoint3D2D(const tvec3<T>& p, int skipCoor)
  {
    const int X = (skipCoor == 0) ? 1 : 0;
    const int Y = (skipCoor == 2) ? 1 : 2;
    return tvec2<T>(p[X], p[Y]);
  }


  //! Point in polygon test in 2D.
  //!
  //! \param[in] p The point to test.
  //! \param[in] n number of vertices.
  //! \param[in] v Random access iterator referencing the vertices of the polygon.
  //! \return \c false if rejected.
  //!
  template<typename T, typename IT>
  inline bool pointInPolygon(const tvec2<T>& p, const int n, IT v)
  {
    bool inside = false;
    const T* e0 = (T*)&v[n-1];
    const T* e1 = (T*)&v[0];
    bool y0 = (e0[1] >= p[1]);
    for (int i=1; i<=n; ++i)
    {
      bool y1 = (e1[1] >= p[1]);
      if ((y0 != y1) && (((e1[1]-p[1])*(e0[0]-e1[0]) >= (e1[0]-p[0])*(e0[1]-e1[1])) == y1))
      {
        inside = !inside;
      }
      y0 = y1;
      e0 = e1;
      e1 = (T*)&v[i];
    }
    return inside;
  }


  //! Point in polygon test in 3D.
  //!
  //! Use this function if it is known which coordinate to skip.
  //!
  //! \param[in] p The point to test.
  //! \param[in] n number of vertices.
  //! \param[in] v Random access iterator referencing the vertices of the polygon.
  //! \param[in] skipCoor Coordinate to skip.
  //! \return \c false if rejected.
  //! \sa reducePolygon3D2D
  //!
  template<typename T, typename IT>
  inline bool pointInPolygon(const tvec3<T>& p, const int n, IT v, const int skipCoor)
  {
    // reduce to two dimensions:
    const int X = (skipCoor == 0) ? 1 : 0;
    const int Y = (skipCoor == 2) ? 1 : 2;

    bool inside = false;
    const T* e0 = (T*)&v[n-1];
    const T* e1 = (T*)&v[0];
    bool y0 = (e0[Y] >= p[Y]);
    for (int i=1; i<=n; ++i)
    {
      bool y1 = (e1[Y] >= p[Y]);
      if ((y0 != y1) && (((e1[Y]-p[Y])*(e0[X]-e1[X]) >= (e1[X]-p[X])*(e0[Y]-e1[Y])) == y1))
      {
        inside = !inside;
      }
      y0 = y1;
      e0 = e1;
      e1 = (T*)&v[i];
    }
    return inside;
  }


  //! Point in polygon test in 3D.
  //!
  //! Use this function if the normal of the polygons plane is known.
  //!
  //! \param[in] p The point to test.
  //! \param[in] n number of vertices.
  //! \param[in] v Random access iterator referencing the vertices of the polygon.
  //! \param[in] normal Normal of the polygons plane.
  //! \sa reducePolygon3D2D
  //!
  template<typename T, typename IT>
  inline bool pointInPolygon(const tvec3<T>& p, const int n, IT v, const tvec3<T>& normal)
  {
    return pointInPolygon(p, n, v, skipCoordinate(normal));
  }


  //! Point in polygon test in 3D.
  //!
  //! Use this function if nothing more than the vertices of the polygon is known.
  //!
  //! \param[in] p The point to test. Must lie on the plane of the polygon.
  //! \param[in] n number of vertices.
  //! \param[in] v Random access iterator referencing the vertices of the polygon.
  //!
  template<typename T, typename IT>
  inline bool pointInPolygon(const tvec3<T>& p, const int n, IT v)
  {
    const tvec3<T> normal(cross(v[1] - v[0], v[n-1] - v[0]));
    return pointInPolygon(p, n, v, skipCoordinate(normal));
  }


  //! Calculates the normals of a polygon (2D or 3D).
  //!
  //! \param[in] n number of vertices.
  //! \param[in] V Random access iterator referencing the vertices of the polygon.
  //! \param[out] N Random access iterator referencing the normals of the polygon.
  //!
  //! This function calculates the normals as
  //! \li 2D: N[i] = norm( \ref perpDot ( V[i]-V[i+1] ); i=0..n-1, n=0.
  //! \li 3D: N[i] = norm( Pn x (V[i]-V[i+1]) ); i=0..n-1, n=0, Pn: plane normal.
  //!
  //! Thus the normal at a vertex is a unit vector in the plane of the polygon, perpendicular to the
  //! \ref polygonEdges "edge" and pointing \e outwards the polygon.
  //!
  template<typename IIT, typename OIT>
  inline void polygonNormals(const int n, IIT V, OIT N)
  {
    detail::polygonNormals(n,V,N,iterTrait(V));
  }


  //! Calculates the edges of a polygon (2D or 3D).
  //!
  //! \param[in] n number of vertices.
  //! \param[in] V Random access iterator referencing the vertices of the polygon (2D or 3D).
  //! \param[out] E Random access iterator referencing the edges of the polygon (2D or 3D).
  //!
  //! This function calculates the (CCW!) edge directions E[i] = V[i+1] - V[i]; i=0..n-1, n=0.\n
  //! The actual edge is then given by V[i] + t*E[i], 0 <= t <= 1.
  //!
  template<typename IIT, typename OIT>
  inline void polygonEdges(const int n, IIT V, OIT E)
  {
    for (int i=0; i<n-1; ++i)
    {
      E[i] = V[i+1]-V[i];
    }
    E[n-1] = V[0]-V[n-1];
  }


}   // namespace glmutils


#endif  // glmutils_polygon_hpp
