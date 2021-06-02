//
// GLSL support library
// Hanno Hildenbrandt 2008
//

//! \file texture.hpp Thin wrapper for OpenGL textures.

#ifndef GLSL_TEXTURE_HPP_INCLUDED
#define GLSL_TEXTURE_HPP_INCLUDED

#include "glsl.hpp"
#include "proxy.hpp"


namespace glsl {

  //! \class texture
  //! Thin Wrapper for OpenGL Textures.
  class texture : proxy<texture>
  {
  public:
    //! Constructs 0-texture
    texture();

    //! Constructs texture of type \c target.
    explicit texture(GLenum target);

    //! Takes ownership over OpenGL texture
    texture(GLenum target, GLuint tex);

    ~texture();

    bool isValid() const { return isValid_(); }
    operator GLuint () const { return get_(); }    //!< Cast to underlying OpenGL object.
    GLuint get() const { return get_(); }      //!< Returns the underlying OpenGL object.

    //! ALias to \c ActiveTexture(TEXTURE0 + \c unit).
    static void activate(GLuint unit);

    //! Alias to BindTexture.
    void bind() const;

    //! Alias to UnbindTexture
    void unbind() const;

    //! Same effect as \ref activate; \ref bind.
    void bind(GLuint unit) const;

    //! Same effect as \ref unbind; \ref activate; \ref bind.
    void rebind(GLuint unit) const;

    //! Same effect as \ref activate; \ref unbind.
    void unbind(GLuint unit) const;

    //! WRAP_S <- WRAP_T <- WRAP_R <- \c wrap.
    void set_wrap(GLint wrap) const;

    //! WRAP_S <- \c wrap_s, WRAP_T <- \c wrap_t, WRAP_R <- \c wrap_r.
    void set_wrap(GLint wrap_s, GLint wrap_t, GLint wrap_r) const;

    //! MIN_FILTER <- \c min_filter, MAG_FILTER <- \c mag filter.
    void set_filter(GLint min_filter, GLint mag_filter) const;

    //! Same effect as \c set_wrap(wrap); \c set_filter(min_filter, mak_filter).
    void set_wrap_filter(GLint wrap, GLint min_filter, GLint mag_filter) const;

    void seti(GLenum pname, GLint param) const;          //!< Alias to \c glTexParameteri
    void setf(GLenum pname, GLfloat param) const;        //!< Alias to \c glTexParameterf
    void setfv(GLenum pname, const GLfloat* params) const;    //!< Alias to \c glTexParameterfv

    GLint geti(GLenum pname) const;                //!< Alias to \c glGetTexParameteriv
    GLfloat getf(GLenum pname) const;              //!< Alias to \c glGetTexParameterfv
    void getfv(GLenum pname, GLfloat* params) const;      //!< Alias to \c glGetTexParameterfv

    void getLeveliv(GLint lod, GLenum value, GLint* data) const;  //!< Alias to \c getTexLevelParameteriv
    void getLevelfv(GLint lod, GLenum value, GLfloat* data) const;  //!< Alias to \c getTexLevelParameterfv

    //! Returns the texture target
    GLuint target() const { return target_; }

    //! Swap
    void swap(texture& other);

  private:
    GLuint target_;

  private:
    friend class proxy<texture>;
    static void release(GLuint x) { glDeleteTextures(1, &x); }
  };

}

#endif
