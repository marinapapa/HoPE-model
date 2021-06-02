//
// GLSL support library
// Hanno Hildenbrandt 2008
//

//! \file buffer.hpp OpenGL Buffer Objects.
//! Supports
//! \li \c ARB_copy_buffer
//! \li \c ABR_map_buffer_range
//! \li \c ARB_texture_buffer_object
//! \li \c ARB_transform_feedback
//! \li \c ARB_uniform_buffer_object

#ifndef GLSL_BUFFER_HPP_INCLUDED
#define GLSL_BUFFER_HPP_INCLUDED


#include "glsl.hpp"
#include "proxy.hpp"


namespace glsl {

  class buffer;


  //! Copy buffer sub data, see \c ARB_copy_buffer.
  void copy_sub_data(buffer& src, buffer& dest,
             GLintptr readOffset, GLintptr writeOffset,
             GLsizeiptr size);


  //! \class buffer
  //! Thin Wrapper for OpenGL Buffer Objects.
  //! Supports
  //! \li \c ARB_copy_buffer
  //! \li \c ABR_map_buffer_range
  //! \li \c ARB_texture_buffer_object
  //! \li \c ARB_transform_feedback
  //! \li \c ARB_uniform_buffer_object
  //!
  class buffer : proxy<buffer>
  {
  public:
    buffer() noexcept;

    //! Creates and binds to \c target.
    explicit buffer(GLenum target);

    //! Takes ownership over OpenGL buffer
    buffer(GLenum target, GLuint glBuf);

    //! Releases all resources
    ~buffer();

    bool isValid() const noexcept { return isValid_(); }
    operator GLuint () const { return get(); }    //!< Cast to underlying OpenGL object.
    GLuint get() const;                           //!< Returns the underlying OpenGL object.

    GLenum target() const noexcept { return target_; }    //!< Returns the current target the buffer is bound to,
    GLint64 size() const noexcept;        //!< Returns the size of the buffer.
    GLenum usage() const;        //!< Returns the usage of the buffer.
    GLenum access() const;       //!< Returns the access of the buffer.
    GLint access_flags() const;  //!< Returns the access flags of the buffer.
    bool mapped() const;         //!< Returns \c TRUE if the buffer is mapped.
    void* map_pointer() const;   //!< Returns the pointer to the mapped buffer data.
    GLint64 map_offset() const;  //!< Returns the offset of the mapped buffer.
    GLint64 map_length() const;  //!< Returns the length of the mapped buffer.

    //! Binds the buffer to the given \c target
    void bind(GLenum target);

    //! Unbinds the buffer
    void unbind();

    //! Grows the the buffer to hold minimal \c reqSize bytes
    void resize(GLsizeiptr reqSize, GLenum usage) const;

    //! Feed the buffer with data
    void data(GLsizeiptr size, const void* data, GLenum usage) const;

    //! Feed a subsection of the buffer with data
    void sub_data(GLintptr offset, GLsizeiptr size, const void* data) const;

    //! Reads a subsection of the buffer data
    void get_buffer_sub_data(GLintptr offset, GLsizeiptr size, void* data) const;

    //! Map to read
    const void* map_read(GLbitfield access = 0) const;

    //! Map to write
    void* map_write(GLbitfield access = 0) const;

    //! Map to read-write
    const void* map_read_write(GLbitfield access = 0) const;

    //! Map buffer range read, see \c ABR_map_buffer_range.
    const void* map_range_read(GLintptr offset, GLsizeiptr length, GLbitfield access = 0) const;

    //! Map buffer range write, see \c ABR_map_buffer_range.
    void* map_range_write(GLintptr offset, GLsizeiptr length, GLbitfield access = 0) const;

    //! Map buffer range read-write, see \c ABR_map_buffer_range.
    void* map_range_read_write(GLintptr offset, GLsizeiptr length, GLbitfield access = 0) const;

    //! Flush mapped buffer range, see \c ABR_map_buffer_range.
    void flush_mapped_range(GLintptr offset, GLsizeiptr length) const;

    //! Unmap the buffer.
    bool unmap() const;

    //! Bind buffer range, see \c transform_feedback.
    void bind_range_feedback(GLuint index, GLintptr offset, GLsizeiptr size);

    //! Bind buffer base, see \c transform_feedback.
    void bind_base_feedback(GLuint index);

    //! Bind buffer range, see \c ARB_uniform_buffer_object.
    void bind_range_uniform(GLuint index, GLintptr offset, GLsizeiptr size);

    //! Bind buffer base, see \c ARB_uniform_buffer_object.
    void bind_base_uniform(GLuint index);

    //! Swap
    void swap(buffer& a);

  private:
    GLenum  target_;
    mutable GLint64 size_;

  private:
    friend class proxy<buffer>;
    static void release(GLuint x) noexcept { glDeleteBuffers(1, &x); }
  };

}   // namespace glsl


#endif



