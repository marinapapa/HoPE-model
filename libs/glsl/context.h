#ifndef GLSL_CONTEXT_H_INCLUDED
#define GLSL_CONTEXT_H_INCLUDED


#include "glsl.hpp"
#include "debug.h"


#ifdef _WIN32 
  #include "wgl_context.hpp"
#else
  #error "Unsupported platform"
#endif


#endif
