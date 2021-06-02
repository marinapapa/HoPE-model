#include "debug.h"
#include <set>
#include <cstdio>
#include <mutex>


namespace glsl
{

  // aux function to translate source to string
  const char*  getStringForSource(GLenum source) 
  {
    switch(source) {
    case GL_DEBUG_SOURCE_API: 
      return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION:
      return "Application";
    case GL_DEBUG_SOURCE_OTHER:
      return "Other";
    default:
      return "Unknown";
    }
  }

  // aux function to translate severity to string
  const char*  getStringForSeverity(GLenum severity) 
  {
    switch(severity) {
    case GL_DEBUG_SEVERITY_HIGH: 
      return "High";
    case GL_DEBUG_SEVERITY_MEDIUM:
      return "Medium";
    case GL_DEBUG_SEVERITY_LOW:
      return "Low";
    default:
      return "Notification";
    }
  }

  // aux function to translate type to string
  const char* getStringForType(GLenum type) 
  {
    switch(type) {
    case GL_DEBUG_TYPE_ERROR: 
      return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY:
      return "Portability Issue";
    case GL_DEBUG_TYPE_PERFORMANCE:
      return "Performance Issue";
    case GL_DEBUG_TYPE_OTHER:
      return "Other";
    default:
      return "Unknown";
    }
  }


  void __stdcall GLDebugLogStdErr(GLenum source, GLenum type, GLuint id, GLenum severity,
                                  GLsizei length, const GLchar* message, const GLvoid*)
  {
    static std::mutex mutex;
    std::lock_guard<std::mutex> _(mutex);
    std::fprintf(stderr, "\nOpenGL log (%d):  Source: %s  Severity: %s\n%s\n", 
                 id, getStringForSource(source), getStringForSeverity(severity), message);
  }


  void __stdcall GLDebugLogOnceStdErr(GLenum source, GLenum type, GLuint id, GLenum severity,
                                      GLsizei length, const GLchar* message, const GLvoid*)
  {
    static std::set<std::string> set;
    auto ip = set.insert(message);
    if (ip.second) {
      GLDebugLogStdErr(source, type, id, severity, length, message, nullptr);
    }
  }


  void SetDebugCallback(GLSL_DEBUG_MSG_LEVEL level, GLDEBUGPROC callback, void* userData)
  {
    auto ilevel = static_cast<int>(level);
    glDebugMessageCallback(callback, userData);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, (ilevel > 1) ? GL_TRUE : GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, (ilevel > 2) ? GL_TRUE : GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, (ilevel > 3) ? GL_TRUE : GL_FALSE);
  };

}

