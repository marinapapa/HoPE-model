#include <utility>        // std::swap
#include <cassert>
#include "texture.hpp"


namespace glsl {

  namespace {
    GLuint genTex() { GLuint x; glGenTextures(1, &x); return x; }
  }

  texture::texture()
  :  target_(0)
  {
  }

  texture::texture(GLenum target)
  :   proxy<texture>(genTex()), target_(target)
  {
  }

  texture::texture(GLenum target, GLuint tex)
  :   proxy<texture>(tex), target_(target)
  {
  }

  texture::~texture()
  {
  }

  void texture::set_wrap(GLint wrap) const
  {
    set_wrap(wrap, wrap, wrap);
  }

  void texture::set_wrap(GLint wrap_s, GLint wrap_t, GLint wrap_r) const
  {
    glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(target_, GL_TEXTURE_WRAP_R, wrap_r);
  }

  void texture::set_filter(GLint min_filter, GLint mag_filter) const
  {
    glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, min_filter);
  }

  void texture::set_wrap_filter(GLint wrap, GLint min_filter, GLint mag_filter) const
  {
    set_wrap(wrap, wrap, wrap);
    set_filter(min_filter, mag_filter);
  }

  void texture::seti(GLenum pname, GLint param) const
  {
    glTexParameteri(target_, pname, param);
  }

  void texture::setf(GLenum pname, GLfloat param) const
  {
    glTexParameterf(target_, pname, param);
  }

  void texture::setfv(GLenum pname, const GLfloat* params) const
  {
    glTexParameterfv(target_, pname, params);
  }

  GLint texture::geti(GLenum pname) const
  {
    GLint param[4]; glGetTexParameteriv(target_, pname, param);
    return param[0];
  }

  GLfloat texture::getf(GLenum pname) const
  {
    GLfloat param[4]; glGetTexParameterfv(target_, pname, param);
    return param[0];
  }

  void texture::getfv(GLenum pname, GLfloat* params) const
  {
    glGetTexParameterfv(target_, pname, params);
  }

  void texture::getLeveliv(GLint lod, GLenum value, GLint* data) const
  {
    glGetTexLevelParameteriv(target_, lod, value, data);
  }

  void texture::getLevelfv(GLint lod, GLenum value, GLfloat* data) const
  {
    glGetTexLevelParameterfv(target_, lod, value, data);
  }

  void texture::activate(GLuint unit)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
  }

  void texture::bind() const
  {
    glBindTexture(target_, get());
  }

  void texture::unbind() const
  {
    glBindTexture(target_, 0);
  }

  void texture::bind(GLuint unit) const
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    bind();
  }

  void texture::rebind(GLuint unit) const
  {
    unbind();
    glActiveTexture(GL_TEXTURE0 + unit);
    bind();
  }

  void texture::unbind(GLuint unit) const
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    unbind();
  }

  void texture::swap(texture& other)
  {
    swap_(other);
    std::swap(target_, other.target_);
  }


}
