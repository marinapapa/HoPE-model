/*
  glad.c
  OpenGL version selector 
  Hanno 2016
*/

#if GLSL_OPENGL_MAJOR_VERSION == 2
  #if GLSL_OPENGL_MINOR_VERSION == 0
    #include "2_0/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 1
    #include "2_1/glad.c"
  #endif
#elif GLSL_OPENGL_MAJOR_VERSION == 3
  #if GLSL_OPENGL_MINOR_VERSION == 0
    #include "3_0/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 1
    #include "3_1/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 2
    #include "3_2/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 3
    #include "3_3/glad.c"
  #endif
#elif GLSL_OPENGL_MAJOR_VERSION == 4
  #if GLSL_OPENGL_MINOR_VERSION == 0
    #include "4_0/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 1
    #include "4_1/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 2
    #include "4_2/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 3
    #include "4_3/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 4
    #include "4_4/glad.c"
  #elif GLSL_OPENGL_MINOR_VERSION == 5
    #include "4_5/glad.c"
  #endif
#endif
#ifndef __gl_h_
  #error "Please define GLSL_OPENGL_MAJOR_VERSION and GLSL_OPENGL_MINOR_VERSION"
#endif
