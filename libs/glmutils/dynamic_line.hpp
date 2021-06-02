//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//


//! \file dynamic_line.hpp Ray-dynamic Line Intersection
//! \ingroup intersection ray


#ifndef glmutils_dynamic_line_hpp
#define glmutils_dynamic_line_hpp


#include <limits>
#include <glm/glm.hpp>
#include <glmutils/perp_dot.hpp>


namespace glmutils {

    using namespace glm;


  //! Represents a Dynamic Line
  //! \ingroup intersection ray
  //!
  //! A dynamic line \f$ PQ(t,s) = P(t) + s \cdot (Q(t) - P(t)) \f$
  //! is the line segment between two points
  //! \f$ P(t) \f$ and \f$ Q(t) \f$ with \f$ PQ(t,0) = P(t) \f$ and
  //! \f$ PQ(t,1) = Q(t) \f$
  //!
  template<typename T>
  class _xdynamicLine
  {
  public:
    //! Default ctor (undefined result)
    //!
    _xdynamicLine() {}

    //! Constructs a Dynamic Line from the endpoints.
    //!
    //! \param P0 P(t=0)
    //! \param P1 P(t=1)
    //! \param Q0 Q(t=0)
    //! \param Q1 Q(t=1)
    //! \param eps (optional) threshold for zero denominator
    //!
    _xdynamicLine(const tvec2<T>& P0, const tvec2<T>& P1, const tvec2<T>& Q0, const tvec2<T>& Q1, const T eps = T(0.00000001))
      : p0(P0), dp(P1-P0), u0(Q0-P0), du((Q1-Q0)-dp)
    {
      a = T(4)*perpDot(u0,du);
      zeroDenom = abs(a) < eps;
      f = T(2) / a;
      na = u0.x + u0.y;
      da = -(du.x + du.y);
    }

    //! Returns PQ(t,s).
    //!
    tvec2<T> eval(T t, T s) const { return (p0+t*dp) + s*(u0 + t*du); }

    //! Returns P(t).
    //!
    tvec2<T> evalP(T t) const { return p0 + t*dp; }

    //! Returns Q(t).
    //!
    tvec2<T> evalQ(T t) const { return evalP(t) + evalPQ(t); }

    //! Returns the edge PQ(t) = Q(t)-P(t).
    //!
    tvec2<T> evalPQ(T t) const { return u0 + t*du; }

    //! Returns the normal of the edge PQ(t), assuming an
    //! polygon line segment with positive winding P->Q.
    //!
    tvec2<T> evalN(T t) const { return -perpDot(normalize(evalPQ(t))); }

    //! Returns the tangent of the edge PQ(t)
    //!
    tvec2<T> evalT(T t) const { return normalize(evalPQ(t)); }

    //! Returns the constant local velocity of the point PQ(t,s) of the edge.
    //! V(s) = PQ(1,s) - PQ(0,s)
    //!
    tvec2<T> evalV(T s) const { return dp + s*du; }

  public:
    tvec2<T>  p0, dp, u0, du;
    T      a, f;
    T      na, da;
    bool    zeroDenom;
  };


  //! ray-dynamic-line intersection.
  //!
  //! \param[in] r0 Ray position at t=0.
  //! \param[in] dr Direction of the ray.
  //! \param[in] DL The dynamic Line.
  //! \param[in] tMin Minimal time.
  //! \param[in] tMax Maximal time.
  //! \param[out] t Time of intersection.
  //! \param[out] s Parameter of intersection 0 <= s <= 1.
  //!
  //! \return \c true if intersection occurs in the time span [tMin,tMax]
  //!
  //! If the function returns \c true the point of intersection is given by:
  //! \f[ r_0 + t \cdot dr = PQ(s,t) = DL.eval(s,t) \f]
  //!
  template<typename T>
  inline bool intersectRayDynamicLine(const tvec2<T>& r0, const tvec2<T>& dr,
                    const _xdynamicLine<T>& DL,
                    T tMin, T tMax,
                    T& t, T& s)
  {
    tvec2<T> X1(DL.dp-dr);
    tvec2<T> X2(r0-DL.p0);
    T b = perpDot(X1,DL.u0) + perpDot(X2,DL.du) - tMin*perpDot(dr,DL.du);  // negated
    T c = perpDot(X1,X2) + tMin*perpDot(dr,DL.dp);
    T s0, s1;
    if (DL.zeroDenom)
    {
      s0 = s1 = c/b;
    }
    else
    {
      T rad = b*b - DL.a*c;
      if (rad < T(0))
      {
        return false;
      }
      rad = std::sqrt(rad);
      s0 = (b + rad) * DL.f;
      s1 = (b - rad) * DL.f;
    }
    T db = X1.x + X1.y;
    T nb = X2.x + X2.y - tMin*(dr.x + dr.y);
    t = std::numeric_limits<T>::max();
    if (s0 >= T(0) && s0 <= T(1))
    {
      T t0 = (s0*DL.na - nb) / (s0*DL.da - db);
      if (t0 >= tMin) { t = t0; s = s0; }
    }
    if (s1 >= T(0) && s1 <= T(1))
    {
      T t1 = (s1*DL.na - nb) / (s1*DL.da - db);
      if (t1 >= tMin && t1 < t) { t = t1; s = s1; }
    }
    return t <= tMax;
  }


  //! ray-dynamic-line intersection, assuming 0<=t<=1
  //!
  //! \param[in] r0 Ray position at t=0.
  //! \param[in] dr Direction of the ray.
  //! \param[in] DL The dynamic Line.
  //! \param[out] t Time of intersection.
  //! \param[out] s Parameter of intersection 0 <= s <= 1.
  //!
  //! \return \c true if intersection occurs in the time span [0,1]
  //!
  //! If the function returns \c true the point of intersection is given by:
  //! \f[ r_0 + t \cdot dr = PQ(s,t) = DL.eval(s,t) \f]
  //!
  template<typename T>
  inline bool intersectRayDynamicLine(const tvec2<T>& r0, const tvec2<T>& dr,
                    const _xdynamicLine<T>& DL,
                    T& t, T& s)
  {
    tvec2<T> X1(DL.dp-dr);
    tvec2<T> X2(r0-DL.p0);
    T b = perpDot(X1,DL.u0) + perpDot(X2,DL.du);  // negated
    T c = perpDot(X1,X2);
    T s0, s1;
    if (DL.zeroDenom)
    {
      s0 = s1 = c/b;
    }
    else
    {
      T rad = b*b - DL.a*c;
      if (rad < T(0))
      {
        return false;
      }
      rad = std::sqrt(rad);
      s0 = (b + rad) * DL.f;
      s1 = (b - rad) * DL.f;
    }
    T db = X1.x + X1.y;
    T nb = X2.x + X2.y;
    t = std::numeric_limits<T>::max();
    if (s0 >= T(0) && s0 <= T(1))
    {
      T t0 = (s0*DL.na - nb) / (s0*DL.da - db);
      if (t0 >= T(0)) { t = t0; s = s0; }
    }
    if (s1 >= T(0) && s1 <= T(1))
    {
      T t1 = (s1*DL.na - nb) / (s1*DL.da - db);
      if (t1 >= T(0) && t1 < t) { t = t1; s = s1; }
    }
    return t <= T(1);
  }


  /**
    \brief ray-dynamic-line intersection.

   Calls \ref intersectRayDynamicLine(const tvec2<T>& r0, const tvec2<T>& dr, const _xdynamicLine<T>& DL, T tMin, T tMax, T& t, T& s, eps)
   "intersectDynamicLine(r0, dr, _xdynaminLine(p0,p1,q0,q1), tMin, tMax, t, s, eps)"
  **/
  template<typename T>
  inline bool intersectRayDynamicLine(const tvec2<T>& r0, const tvec2<T>& dr,
                    const tvec2<T>& p0, const tvec2<T>& p1,
                    const tvec2<T>& q0, const tvec2<T>& q1,
                    T tMin, T tMax,
                    T& t, T& s,
                    const T eps = T(0.00000001))
  {
    _xdynamicLine<T> DL(p0, p1, q0, q1);
    return intersectRayDynamicLine(r0, dr, DL, tMin, tMax, t, s, eps);
  }


  /**
    \brief ray-dynamic-line intersection, assuming 0<=t<=1.

   Calls \ref intersectRayDynamicLine(const tvec2<T>& r0, const tvec2<T>& dr, const _xdynamicLine<T>& DL, T& t, T& s, eps)
   "intersectDynamicLine(r0, dr, _xdynaminLine(p0,p1,q0,q1), s, t, eps)"
  **/
  template<typename T>
  inline bool intersectRayDynamicLine(const tvec2<T>& r0, const tvec2<T>& dr,
                    const tvec2<T>& p0, const tvec2<T>& p1,
                    const tvec2<T>& q0, const tvec2<T>& q1,
                    T& t, T& s,
                    const T esp = T(0.0000001))
  {
    _xdynamicLine<T> DL(p0, p1, q0, q1);
    return intersectRayDynamicLine(r0, dr, DL, t, s);
  }


  typedef _xdynamicLine<float> dynamicLine;    //!< Dynamic Line of vec2
  typedef _xdynamicLine<double> ddynamicLine;    //!< Dynamic Line of dvec2


}   // namespace glmutils


#endif  // glmutils_dynamic_line_hpp
