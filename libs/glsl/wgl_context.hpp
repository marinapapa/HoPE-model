//
// GLSL support library
// Hanno Hildenbrandt 2008
//

//! \file wgl_context.hpp WGL context creation

#ifndef _WIN32 
  #error wgl not supported
#endif

#ifndef GLSL_WGL_CONTEXT_HPP_INCLUDED
#define GLSL_WGL_CONTEXT_HPP_INCLUDED


#define WIN32_MEAN_AND_LEAN
#include <glad\glad_wgl.h>


namespace glsl {

  class Context
  {
  public:
    Context(Context&&);
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Context();
    explicit Context(::HWND hWnd);
    Context(::HWND hWnd, int colorSamples, int coverageSamples, int* attributes = nullptr);
    ~Context();

    void MakeCurrent();
    void ReleaseCurrent();
    void SwapBuffers();
    void SwapInterval(int);

  private:
    ::HWND hWnd_;
    ::HDC hDC_;
    ::HGLRC hGLRC_;
  };
  
}


#endif
