//
// Utilities for OpenGL Mathematics (glm)
//
// Hanno Hildenbrandt 2008
//

//! \defgroup traits Generic programming
//! \file traits.hpp Traits for vector and matrixes
//! \ingroup traits


#ifndef glmutils_traits_hpp
#define glmutils_traits_hpp


#include <glm/glm.hpp>
#include <iterator>


namespace glmutils {


/** Generic  %trait class for glm types.

\struct trait
Specialization for vector types:
\code
template<typename VEC>
struct trait {
  underlying numeric type            value_type
  number of columns              columns = 1
  number of rows                rows
  column length                col_size
  row length                  row_size = 1
  number of elements              size
  type of vector with different numeric type  other<U>::type
  vector of length size-1            reduced
  vector of length size+1            homogeneous
  matrix[rows,rows]              square
};
\endcode
\n Specialization for matrix types:
\code
template<typename MAT>
struct trait {
  underlying numeric type            value_type
  type of column vector            col_type
  type of row vector              row_type
  number of columns              columns
  number of rows                rows
  column length                col_size
  row length                  row_size
  number of elements              size
  type of matrix with different numeric type  other<U>::type
  matrix[col_size-1,row_size-1]        reduced
  matrix[rows,columns]            transposed
  matrix[max(rows,colums),max(rows,columns)]  square
};
\endcode
*/
  template<typename X>
  struct trait {};


  //! What glm-type is behind a iterator? See \ref trait
  template<typename IT>
  struct iter_trait
  {
    typedef typename std::iterator_traits<IT>::value_type value_type;  //!< vector or matrix type behind iterator
    typedef typename value_type::value_type elem_type;          //!< value type of the above type
  };

  //! Returns the trait class for the type behind the iterator
  template<typename IT>
  inline trait< typename iter_trait<IT>::value_type > iterTrait(IT)
  {
    return trait< typename iter_trait<IT>::value_type >();
  }


//! \cond DETAIL
#define GLMUTILS_GENERATE_VEC_TRAIT(X,T,C,S,H) \
  template<> struct trait<X<T> >  \
  { \
    typedef T value_type; \
    static const int columns = 1; \
    static const int rows = C; \
    static const int col_size = C; \
    static const int row_size = 1; \
    static const int size = C; \
    template<typename U> struct other { typedef X<U> type; }; \
    typedef S reduced; \
    typedef glm::tvec##H <T> homogeneous; \
    typedef glm::tmat##C##x##C <T> square; \
  } \


#define GLMUTILS_GENERATE_MAT_TRAIT(X,T,C,R,S,N) \
  template<> struct trait<X<T> >    \
  { \
    typedef T value_type; \
    typedef glm::tvec##R <T> col_type; \
    typedef glm::tvec##C <T> row_type; \
    static const int columns = C; \
    static const int rows = R; \
    static const int col_size = R; \
    static const int row_size = C; \
    static const int size = (C) * (R); \
    template<typename U> struct other { typedef X<U> type; }; \
    typedef S reduced; \
    typedef glm::tmat##R##x##C <T> transposed; \
    typedef glm::tmat##N##x##N <T> square; \
  } \

//! \endcond

  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec2,float,2,float,3);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec3,float,3,glm::tvec2<float>,4);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec4,float,4,glm::tvec3<float>,4);

  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec2,double,2,double,3);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec3,double,3,glm::tvec2<double>,4);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec4,double,4,glm::tvec3<double>,4);

  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec2,int,2,int,3);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec3,int,3,glm::tvec2<int>,4);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec4,int,4,glm::tvec3<int>,4);

  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec2,unsigned,2,unsigned,3);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec3,unsigned,3,glm::tvec2<unsigned>,4);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec4,unsigned,4,glm::tvec3<unsigned>,4);

  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec2,short,2,short,3);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec3,short,3,glm::tvec2<short>,4);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec4,short,4,glm::tvec3<short>,4);

  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec2,bool,2,bool,3);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec3,bool,3,glm::tvec2<bool>,4);
  GLMUTILS_GENERATE_VEC_TRAIT(glm::tvec4,bool,4,glm::tvec3<bool>,4);

  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat2x2,float,2,2,float,2);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat3x3,float,3,3,glm::tmat2x2<float>,3);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat4x4,float,4,4,glm::tmat3x3<float>,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat2x3,float,2,3,void,3);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat3x2,float,3,2,void,3);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat2x4,float,2,4,void,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat4x2,float,4,2,void,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat3x4,float,3,4,glm::tmat3x2<float>,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat4x3,float,4,3,glm::tmat3x2<float>,4);

  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat2x2,double,2,2,double,2);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat3x3,double,3,3,glm::tmat2x2<double>,3);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat4x4,double,4,4,glm::tmat3x3<double>,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat2x3,double,2,3,void,3);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat3x2,double,3,2,void,3);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat2x4,double,2,4,void,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat4x2,double,4,2,void,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat3x4,double,3,4,glm::tmat2x3<double>,4);
  GLMUTILS_GENERATE_MAT_TRAIT(glm::tmat4x3,double,4,3,glm::tmat3x2<double>,4);

}   // namespace glmutils

#endif  // glmutils_traits_hpp
