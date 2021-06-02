#ifndef GLSL_IMM_HPP_INCLUDED
#define GLSL_IMM_HPP_INCLUDED

#include <vector>
#include <atomic>
#include <glm/glm.hpp>
#include <glmutils/bbox.hpp>
#include "glsl.hpp"


namespace glsl {

  class color32
  {
  public:
    color32() noexcept {}
    explicit color32(const glm::vec4& cf) noexcept : cub(cast(cf.x, cf.y, cf.z, cf.w)) {}
    explicit color32(const glm::vec3& cf) noexcept : cub(cast(cf.x, cf.y, cf.z, 1.0f)) {}
    color32(const glm::vec3& cf, float alpha) noexcept : cub(cast(cf.x, cf.y, cf.z, alpha)) {}
    color32(float r, float g, float b) noexcept : cub(cast(r, g, b, 1.0f)) {}
    color32(float r, float g, float b, float a) noexcept : cub(cast(r, g, b, a)) {}

  private:
    constexpr static uint32_t cast(float r, float g, float b, float a) noexcept
    {
      return uint32_t(255.f * r) | (uint32_t(255.f * g) << 8) | (uint32_t(255.f * b) << 16) | (uint32_t(255.f * a) << 24);
    }
    uint32_t cub;
  };


  const color32 defaultColor(1, 1, 1, 1);
  const float ignoreTexCoord(100.0f);
  const float discardTexCoord(-100.0f);


  enum IMMprimitive
  {
    IMM_POINT = 0,
    IMM_LINES,
    IMM_LINE_STRIP,
    IMM_TRIANGLES,
    IMM_TRIANGLE_STRIP,
    IMM_FILLED_TRIANGLES,
    IMM_FILLED_TRIANGLE_STRIP,
    IMM_MAX_PRIMITIVE
  };


  class Imm
  {
    static const GLenum IMM_GL_MAP[IMM_MAX_PRIMITIVE];

  public:
    struct T1_V3_C4
    {
      T1_V3_C4() noexcept {}
      T1_V3_C4(const glm::vec3& vert, const color32& color, float tex)
        : t1(tex), v3(vert), c4(color) {}

      glm::vec3 v3;
      float     dummy;
      float     t1;
      color32   c4;
    };

  public:
    Imm();
    ~Imm();
    void Begin(IMMprimitive Primitive);
    void Emit(const glm::vec3& vert, const color32& color, float tex);
    void Emit(const glm::vec3& vert, float tex) { Emit(vert, defaultColor, tex); }
    void Emit(const glm::vec3& vert, const color32& color) { Emit(vert, color, ignoreTexCoord); }
    void Emit(const glm::vec2& vert, const color32& color) { Emit(glm::vec3(vert, 0.0f), color, ignoreTexCoord); }
    void Emit(const glm::vec2& vert, float tex) { Emit(glm::vec3(vert, 0.0f), defaultColor, tex); }
    void Emit(const glm::vec2& vert) { Emit(glm::vec3(vert, 0.0f), defaultColor, ignoreTexCoord); }
    void Emit(float x, float y, float z, const color32& color, float tex) { Emit(glm::vec3(x, y, z), color, tex); }
    void Emit(float x, float y, float z, const color32& color) { Emit(x, y, z, color, ignoreTexCoord); }
    void Emit(float x, float y, float z, float tex) { Emit(x, y, z, defaultColor, tex); }
    void Emit(float x, float y, const color32& color, float tex) { Emit(x, y, 0.0f, color, tex); }
    void Emit(float x, float y, const color32& color) { Emit(x, y, 0.0f, color, ignoreTexCoord); }
    void Emit(float x, float y, float tex) { Emit(x, y, 0.0f, defaultColor, tex); }
    void End();

    void Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex);
    void Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex);
    void Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color);
    void Box(const glm::vec3& p0, const glm::vec3& p1, float ctex);
    void Box(const glmutils::bbox3& box, const color32& color);
    void Box(const glmutils::bbox3& box, float ctex);
    void Box(const glm::mat4 M, const glm::vec3& p0, const glm::vec3& p1, const color32& color);
    void Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, float ctex);
    void Box(const glm::mat4& M, const glmutils::bbox3& box, const color32& color);
    void Box(const glm::mat4& M, const glmutils::bbox3& box, float ctex);

    void Flush();
    void Render();
    void RenderTorus(float WH);

  private:
    void resize_buffer(size_t n);

    IMMprimitive primitive = IMM_POINT;
    size_t start = 0;
    std::vector<T1_V3_C4> attribs[IMM_MAX_PRIMITIVE];
    GLuint vbo_ = GL_NONE;
    void* pbuf_ = nullptr;
    size_t buf_size_ = 0;
    GLuint vao_ = GL_NONE;
    GLuint prog_ = GL_NONE;
    GLuint progTorus_ = GL_NONE;
    std::atomic<size_t> tot_primitives_ = 0;
    size_t primitives_[IMM_MAX_PRIMITIVE];
  };

}

#endif
