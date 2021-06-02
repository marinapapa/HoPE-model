//
// GLSL support library
// Hanno Hildenbrandt 2008
//

#include "vertexarray.hpp"


namespace glsl {

  vertexarray::vertexarray()
  {
  }

  vertexarray::~vertexarray()
  {
  }

  void vertexarray::bind()
  {
    GLuint h = get_();
    if (0 == h) { glGenVertexArrays(1, &h); this->reset_(h); }
    glBindVertexArray(h);
  }

  void vertexarray::unbind()
  {
    glBindVertexArray(0);
  }


  void vertexarray::swap(vertexarray& other)
  {
    swap_(other);
  }


}  // namespace glsl
