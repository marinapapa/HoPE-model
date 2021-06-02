//
// GLSL support library
// Hanno Hildenbrandt 2008
//

//! \file vertexarray.hpp OpenGL Vertex Array Objects.
//! Supports
//! \li \c ARB_vertex_array_object

#ifndef GLSL_VERTEXARRAY_HPP_INCLUDED
#define GLSL_VERTEXARRAY_HPP_INCLUDED

#include "glsl.hpp"
#include "proxy.hpp"


namespace glsl {


  //! \class vertexarray
  //! Thin Wrapper for OpenGL Vertex Array Objects.
  //! Supports
  //! \li \c ARB_vertex_array_object
  //!
  class vertexarray : proxy<vertexarray>
  {
  public:
    vertexarray();
    ~vertexarray();

    bool isValid() const { return isValid_(); }
    operator GLuint (void) const { return get_(); }  //!< Cast to OpenGL name
    GLuint name() const { return get_(); }      //!< Returns OpenGL name

    //! Binds the vertex array
    void bind();

    //! Unbinds the vertex array
    void unbind();

    //! Swap
    void swap(vertexarray& other);

  private:
    friend class proxy<vertexarray>;
    static void release(GLuint x) { glDeleteVertexArrays(1, &x); }
  };

}   // namespace glsl


#endif



