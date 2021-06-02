//
// GLSL support library
// Hanno Hildenbrandt 2008
//

#include <cassert>
#include "buffer.hpp"


namespace glsl {

  namespace {
    GLuint gen_buffer() noexcept { GLuint b; glGenBuffers(1, &b); return b; }
  }

  buffer::buffer() noexcept
  : target_(0), size_(0)
  {
  }

  buffer::buffer(GLenum target)
  :  proxy<buffer>(gen_buffer()), target_(target), size_(0)
  {
    glBindBuffer(target_, get_());
  }

  buffer::buffer(GLenum target, GLuint glBuf)
  : proxy<buffer>(glBuf), target_(target), size_(0)
  {
    glBindBuffer(target_, get_());
  }


  buffer::~buffer()
  {
  }

  GLuint buffer::get() const
  {
    GLuint h = get_();
    if (0 == h) { glGenBuffers(1, &h); this->reset_(h); }
    return h;
  }

  GLint64 buffer::size() const noexcept
  {
    return size_;
  }

  GLenum buffer::usage() const
  {
    GLint ret;
    glGetBufferParameteriv(target_, GL_BUFFER_USAGE, &ret);
    return GLenum(ret);
  }

  GLenum buffer::access() const
  {
    GLint ret;
    glGetBufferParameteriv(target_, GL_BUFFER_ACCESS, &ret);
    return GLenum(ret);
  }

  GLint buffer::access_flags() const
  {
    GLint ret;
    glGetBufferParameteriv(target_, GL_BUFFER_ACCESS_FLAGS, &ret);
    return ret;
  }

  bool buffer::mapped() const
  {
    GLint ret;
    glGetBufferParameteriv(target_, GL_BUFFER_MAPPED, &ret);
    return (GL_TRUE == ret);
  }

  void* buffer::map_pointer() const
  {
    void* ret;
    glGetBufferPointerv(target_, GL_BUFFER_MAP_POINTER, &ret);
    return ret;
  }

  GLint64 buffer::map_offset() const
  {
    GLint64 ret;
    glGetBufferParameteri64v(target_, GL_BUFFER_MAP_OFFSET, &ret);
    return ret;
  }

  GLint64 buffer::map_length() const
  {
    GLint64 ret;
    glGetBufferParameteri64v(target_, GL_BUFFER_MAP_LENGTH, &ret);
    return ret;
  }


  void buffer::bind(GLenum target)
  {
    glBindBuffer(target_=target, get());
  }

  void buffer::unbind()
  {
    glBindBuffer(target_, 0);
    target_ = 0;
  }

  void buffer::resize(GLsizeiptr reqSize, GLenum usage) const
  {
    if (size_ < reqSize) {
      data(reqSize, nullptr, usage);
    }
  }

  void buffer::data(GLsizeiptr size, const void* data, GLenum usage) const
  {
    glBufferData(target_, size, data, usage);
    size_ = size;
  }

  void buffer::sub_data(GLintptr offset, GLsizeiptr size, const void* data) const
  {
    glBufferSubData(target_, offset, size, data);
  }

  void buffer::get_buffer_sub_data(GLintptr offset, GLsizeiptr size, void* data) const
  {
    glGetBufferSubData(target_, offset, size, data);
  }

  const void* buffer::map_read(GLbitfield access) const
  {
    return glMapBuffer(target_, access | GL_READ_ONLY);
  }

  void* buffer::map_write(GLbitfield access) const
  {
    return glMapBuffer(target_, access | GL_WRITE_ONLY);
  }

  const void* buffer::map_read_write(GLbitfield access) const
  {
    return glMapBuffer(target_, access | GL_READ_WRITE);
  }

  const void* buffer::map_range_read(GLintptr offset, GLsizeiptr length, GLbitfield access) const
  {
    return glMapBufferRange(target_, offset, length, access | GL_MAP_READ_BIT);
  }

  void* buffer::map_range_write(GLintptr offset, GLsizeiptr length, GLbitfield access) const
  {
    return glMapBufferRange(target_, offset, length, access | GL_MAP_WRITE_BIT);
  }

  void* buffer::map_range_read_write(GLintptr offset, GLsizeiptr length, GLbitfield access) const
  {
    return glMapBufferRange(target_, offset, length, access | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
  }

  void buffer::flush_mapped_range(GLintptr offset, GLsizeiptr length) const
  {
    glFlushMappedBufferRange(target_, offset, length);
  }

  void buffer::bind_range_feedback(GLuint index, GLintptr offset, GLsizeiptr size)
  {
    glBindBufferRange(target_ = GL_TRANSFORM_FEEDBACK_BUFFER, index, get(), offset, size);
  }

  void buffer::bind_base_feedback(GLuint index)
  {
    glBindBufferBase(target_ = GL_TRANSFORM_FEEDBACK_BUFFER, index, get());
  }

  void buffer::bind_range_uniform(GLuint index, GLintptr offset, GLsizeiptr size)
  {
    glBindBufferRange(target_ = GL_UNIFORM_BUFFER, index, get(), offset, size);
  }

  void buffer::bind_base_uniform(GLuint index)
  {
    glBindBufferBase(target_ = GL_UNIFORM_BUFFER, index, get());
  }

  bool buffer::unmap() const
  {
    return GL_TRUE == glUnmapBuffer(target_);
  }


  void buffer::swap(buffer& a)
  {
    swap_(a);
    std::swap(target_, a.target_);
  }


  void copy_sub_data(buffer& src, buffer& dest,
             GLintptr readOffset, GLintptr writeOffset,
             GLsizeiptr size)
  {
    glCopyBufferSubData(src.target(), dest.target(), readOffset, writeOffset, size);
  }

}   // namespace glsl



