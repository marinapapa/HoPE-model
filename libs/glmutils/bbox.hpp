//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup bbox Bounding boxes
//! \file bbox.hpp Axis-aligned bounding boxes (min-max representation)
//! \ingroup bbox


#ifndef glmutils_bbox_hpp
#define glmutils_bbox_hpp

#include <glm/glm.hpp>
#include <glmutils/traits.hpp>


namespace glmutils {

  using namespace glm;  
  using namespace glm::detail;

  template<typename bbox>
  inline typename bbox::vec_type center(const bbox& b) noexcept;

  template<typename bbox>
  inline typename bbox::value_type volume(const bbox& b) noexcept;

  template<typename bbox>
  inline typename bbox::vec_type extent(const bbox& b) noexcept;

  template<typename bbox>
  inline void inflate(bbox& b, const typename bbox::vec_type& dv) noexcept;

  template<typename bbox>
  inline void include(bbox& lhs, const typename bbox::vec_type& rhs) noexcept;

  template<typename bbox>
  inline void include(bbox& lhs, const bbox& rhs) noexcept;

  template <typename bbox>
  inline typename bbox::vec_type clip(typename bbox::vec_type const&, bbox const& b) noexcept;

  template <typename bbox>
  inline bbox clip(bbox const& a, bbox const& b) noexcept;
  

  //! Includes \a n points in \a rhs to a bounding box.
  //! \param[in] lhs Destination bounding box
  //! \param[in] n Number of points to include
  //! \param[in] rhs Forward iterator referencing to \a bbox::vec_type
  //!
  template<typename bbox, typename IT>
  inline void include(bbox& lhs, const int n, IT rhs);


  //! \brief Generic axis-aligned bounding box
  //! \tparam V Underlying vector type
  template<typename V>
  class tbbox
  {
    V p_[2];

  public:
    typedef V vec_type;                    //!< The underlying vector type
    typedef typename trait<V>::value_type value_type;    //!< The underlying numeric type

    tbbox() noexcept {}                          //!< Default constructor. Result is undefined.
    explicit tbbox(const vec_type& v) noexcept { p_[0] = p_[1] = v; }  //!< Constructs a empty bounding box with center \a v.

    template<typename O>
    explicit tbbox(const tbbox<O>& box) noexcept { p_[0] = vec_type(box.p0()); p_[1] = vec_type(box.p1()); }

    //! Constructs a bounding box including a set of points.
    //! \param[in] n Number of points.
    //! \param[in] v Forward iterator referencing to \a vec_type
    template<typename IT>
    tbbox(const int n, IT v) noexcept
    {
      p_[0] = p_[1] = v[0];
      include(*this, n-1, v+1);
    }

    //! Constructs a rectangular bounding box with center \a v and radius \a r.
    tbbox(const vec_type& v, value_type r) noexcept { p_[0] = (v-r);  p_[1] = (v+r); }

    //! Constructs a bounding box from two corners.
    tbbox(const vec_type& p0, const vec_type& p1) noexcept
    {
      p_[0] = min<value_type>(p0, p1);
      p_[1] = max<value_type>(p0, p1);
    }

    //! Assignment
    tbbox<vec_type>& operator=(const tbbox<vec_type>& b) noexcept
    {
      p_[0] = b.p_[0];
      p_[1] = b.p_[1];
      return *this;
    }

    const vec_type& operator[](int i) const noexcept { return p_[i]; }  //!< Access to corner \a i.
    vec_type& operator[](int i) noexcept { return p_[i]; }        //!< Access to corner \a i.

    vec_type& p0() noexcept { return p_[0]; }              //!< Returns min. corner.
    vec_type& p1() noexcept { return p_[1]; }              //!< Returns max. corner.
    const vec_type& p0() const noexcept { return p_[0]; }        //!< Returns min. corner.
    const vec_type& p1() const noexcept { return p_[1]; }        //!< Returns max. corner.
  };


  //! Returns the center of a bounding box
  template<typename bbox>
  inline typename bbox::vec_type center(const bbox& b) noexcept
  {
    typename bbox::vec_type tmp(b.p0() + (b.p1() - b.p0()) / typename bbox::value_type(2));
    return tmp;
  }


  //! Moves the center of a bounding box
  template<typename bbox>
  inline void move(bbox& b, const typename bbox::vec_type& dp) noexcept
  {
    b.p0() += dp;
    b.p1() += dp;
  }


  //! Returns the volume of a bounding box
  template<typename bbox>
  inline typename bbox::value_type volume(const bbox& b) noexcept
  {
    typename bbox::vec_type ext(b.p1() - b.p0());
    typename bbox::value_type V = ext[0];
    for (int i=1; i<trait<typename bbox::vec_type>::size; ++i)
    {
      V *= ext[i];
    }
    return V;
  }


  //! Returns the extent of a bounding box
  template<typename bbox>
  inline typename bbox::vec_type extent(const bbox& b) noexcept
  {
    return typename bbox::vec_type(b.p1() - b.p0());
  }


  //! Inflates the bounding box.
  template<typename bbox>
  inline void inflate(bbox& b, const typename bbox::vec_type& dv) noexcept
  {
    typedef typename bbox::value_type T;
    b.p0() -= T(0.5) * dv;
    b.p1() += T(0.5) * dv;
  }


  //! Includes the the point \c rhs into the bounding box
  template<typename bbox>
  inline void include(bbox& lhs, const typename bbox::vec_type& rhs) noexcept
  {
    lhs.p0() = min<typename bbox::value_type>(lhs.p0(), rhs);
    lhs.p1() = max<typename bbox::value_type>(lhs.p1(), rhs);
  }


  //! Includes \c n points in \c rhs into the bounding box
  template<typename bbox, typename IT>
  inline void include(bbox& lhs, const int n, IT rhs)
  {
    for (int i=0; i<n; ++i, ++rhs) {
      lhs.p0() = min<typename bbox::value_type>(lhs.p0(), *rhs);
      lhs.p1() = max<typename bbox::value_type>(lhs.p1(), *rhs);
    }
  }


  //! Includes the bounding box \rhs into the bounding box
  template<typename bbox>
  inline void include(bbox& lhs, const bbox& rhs) noexcept
  {
    lhs.p0() = min<typename bbox::value_type>(lhs.p0(), rhs.p0());
    lhs.p1() = max<typename bbox::value_type>(lhs.p1(), rhs.p1());
  }


  //! \returns the point \a pt clipped against bounding box \a b.
  template <typename bbox>
  inline typename bbox::vec_type clip(typename bbox::vec_type const& pt, bbox const& b) noexcept
  {
    return typename bbox::vec_type(min(b.p1().x, max(pt.x, b.p0().x)), min(b.p1().y, max(pt.y, b.p0().y)));
  }


  //! \returns the bounding box \a a clipped against bounding box \a b.
  template <typename bbox>
  inline bbox clip(bbox const& a, bbox const& b) noexcept
  {
    return bbox(clip(a.p0(), b), clip(a.p1(), b));
  }



  typedef tbbox<tvec2<float> > bbox2;      //!< 2D bounding box of floats
  typedef tbbox<tvec3<float> > bbox3;      //!< 3D bounding box of floats
  typedef tbbox<tvec4<float> > bbox4;      //!< 4D bounding box of floats

  typedef tbbox<tvec2<double> > dbbox2;    //!< 2D bounding box of doubles
  typedef tbbox<tvec3<double> > dbbox3;    //!< 3D bounding box of doubles
  typedef tbbox<tvec4<double> > dbbox4;    //!< 4D bounding box of doubles

  typedef tbbox<tvec2<int> > ibbox2;       //!< 2D bounding box of ints
  typedef tbbox<tvec3<int> > ibbox3;       //!< 3D bounding box of ints
  typedef tbbox<tvec4<int> > ibbox4;       //!< 4D bounding box of ints

}   // namespace glmutils



#endif  // glmutils_bbox_hpp
