#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <glm/glm.hpp>
#include "glsl/glsl.hpp"


namespace shader {


  struct uniformBlock_t
  {
    glm::mat4 V;
    glm::mat4 P;
    glm::mat4 MV;
    glm::mat4 MVP;
    glm::mat4 ITMV;
    glm::mat4 Ortho;      // ortho
    glm::mat4 Text;       // font
    float zoom;
  };


  extern const char* uniformBlock;

  extern const char* birdVertex;
  extern const char* birdGeo;
  extern const char* birdFragment;
  
  extern const char* texVertex;
  extern const char* texFragment;
  extern const char* texGeo;
  
  extern const char* immVertexTorus;
  extern const char* immVertex;
  extern const char* immFragment;
  
  extern const char* textVertex;
  extern const char* textGeo;
  extern const char* textFragment;
  
  extern const char* ribbonVertex;
  extern const char* ribbonGeo;
  extern const char* ribbonFragment;


  GLuint ProgFromLiterals(const char* vertexShader, const char* fragmentShader, const char* geometyShader = nullptr);

}

#endif
