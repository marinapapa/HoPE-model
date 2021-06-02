#include <array>
#include <algorithm>
#include <glmutils/homogeneous.hpp>
#include "imm.hpp"
#include <simgl/shader.h>


namespace glsl {

  const int IMM_INIT_BUF_SIZE = 1024;
  const GLenum Imm::IMM_GL_MAP[IMM_MAX_PRIMITIVE] =
  {
    GL_POINTS,
    GL_LINES,
    GL_LINE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP
  };


  Imm::Imm()
  {
    prog_ = shader::ProgFromLiterals(shader::immVertex, shader::immFragment, nullptr);
    progTorus_ = shader::ProgFromLiterals(shader::immVertexTorus, shader::immFragment, nullptr);
    resize_buffer(IMM_INIT_BUF_SIZE);
  }


  Imm::~Imm()
  {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
    glDeleteProgram(prog_);
    glDeleteProgram(progTorus_);
  }


  void Imm::resize_buffer(size_t n)
  {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenVertexArrays(1, &vao_);
    const auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferStorage(GL_ARRAY_BUFFER, n * sizeof(T1_V3_C4), nullptr, flags);
    pbuf_ = glMapBufferRange(GL_ARRAY_BUFFER, 0, n * sizeof(T1_V3_C4), flags);
    buf_size_ = n;
    glBindVertexArray(vao_);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(T1_V3_C4), (void*)(char*)(offsetof(T1_V3_C4, v3)));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T1_V3_C4), (void*)(char*)(offsetof(T1_V3_C4, c4)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(T1_V3_C4), (void*)(char*)(offsetof(T1_V3_C4, t1)));
  }


  void Imm::Emit(const glm::vec3& vert, const color32& color, float tex)
  {
    switch (primitive)
    {
    case IMM_LINE_STRIP:
    case IMM_TRIANGLE_STRIP:
    case IMM_FILLED_TRIANGLE_STRIP:
      if (start == attribs[primitive].size())
      {
        attribs[primitive].emplace_back(vert, color, discardTexCoord);
      }
      break;
    }
    attribs[primitive].emplace_back(vert, color, tex);
  }


  void Imm::Begin(IMMprimitive Primitive)
  {
    primitive = Primitive;
    start = attribs[primitive].size();
  }


  void Imm::End()
  {
    switch (primitive)
    {
    case IMM_LINE_STRIP:
    case IMM_TRIANGLE_STRIP:
    case IMM_FILLED_TRIANGLE_STRIP:
      // Add degenerated vertex
      if (start != attribs[primitive].size())
      {
        attribs[primitive].push_back(attribs[primitive].back());
        attribs[primitive].back().t1 = discardTexCoord;
      }
      break;
    }
  }


  void Imm::Flush()
  {
    tot_primitives_ = 0;
    for (int i = IMM_POINT; i < IMM_MAX_PRIMITIVE; ++i)
    {
      tot_primitives_ += attribs[i].size();
    }
    if (tot_primitives_)
    {
      if (tot_primitives_ >= buf_size_) {
        // buffer overflow. defer handling to cs_render()
        return;
      }
      size_t tpc = 0;
      for (int i = IMM_POINT; i < IMM_MAX_PRIMITIVE; ++i)
      {
        auto pbuf = ((T1_V3_C4*)pbuf_) + tpc;
        primitives_[i] = attribs[i].size();
        if (primitives_[i])
        {
          memcpy(pbuf, attribs[i].data(), attribs[i].size() * sizeof(T1_V3_C4));
          attribs[i].clear();
        }
        tpc += primitives_[i];
      }
    }
  }


  // runs on GUI thread
  void Imm::Render()
  {
    if (tot_primitives_)
    {
      if (tot_primitives_ >= buf_size_) {
        // buffer overflow
        resize_buffer((110 * tot_primitives_) / 100);
        Flush();
      }
      glUseProgram(prog_);
      glBindVertexArray(vao_);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      size_t tpc = 0;
      for (int i = IMM_POINT; i < IMM_MAX_PRIMITIVE; ++i)
      {
        if (i == IMM_FILLED_TRIANGLES) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (primitives_[i])
        {
          glDrawArrays(IMM_GL_MAP[i], (GLint)tpc, (GLsizei)primitives_[i]);
        }
        tpc += primitives_[i];
      }
    }
  }


  // runs on GUI thread
  void Imm::RenderTorus(float WH)
  {
    if (tot_primitives_)
    {
      if (tot_primitives_ >= buf_size_) {
        // buffer overflow
        resize_buffer((110 * tot_primitives_) / 100);
        Flush();
      }
      glUseProgram(progTorus_);
      glUniform3f(0, 0.f, -WH, WH);
      glBindVertexArray(vao_);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      size_t tpc = 0;
      for (int i = IMM_POINT; i < IMM_MAX_PRIMITIVE; ++i)
      {
        if (i == IMM_FILLED_TRIANGLES) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (primitives_[i])
        {
          glDrawArraysInstanced(IMM_GL_MAP[i], (GLint)tpc, (GLsizei)primitives_[i], 9);
        }
        tpc += primitives_[i];
      }
    }
  }


  void Imm::Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex)
  {
    Begin(IMM_LINE_STRIP);
    Emit(glm::vec3(p0.x, p0.y, p0.z), color, ctex);
    Emit(glm::vec3(p1.x, p0.y, p0.z), color, ctex);
    Emit(glm::vec3(p1.x, p1.y, p0.z), color, ctex);
    Emit(glm::vec3(p0.x, p1.y, p0.z), color, ctex);
    Emit(glm::vec3(p0.x, p0.y, p0.z), color, ctex);
    Emit(glm::vec3(p0.x, p0.y, p1.z), color, ctex);
    Emit(glm::vec3(p1.x, p0.y, p1.z), color, ctex);
    Emit(glm::vec3(p1.x, p1.y, p1.z), color, ctex);
    Emit(glm::vec3(p0.x, p1.y, p1.z), color, ctex);
    Emit(glm::vec3(p0.x, p0.y, p1.z), color, ctex);
    End();
    Begin(IMM_LINES);
    Emit(glm::vec3(p1.x, p0.y, p0.z), color, ctex);
    Emit(glm::vec3(p1.x, p0.y, p1.z), color, ctex);
    Emit(glm::vec3(p0.x, p1.y, p0.z), color, ctex);
    Emit(glm::vec3(p0.x, p1.y, p1.z), color, ctex);
    Emit(glm::vec3(p1.x, p1.y, p0.z), color, ctex);
    Emit(glm::vec3(p1.x, p1.y, p1.z), color, ctex);
    End();
  }


  void Imm::Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color)
  {
    Box(p0, p1, color, ignoreTexCoord);
  }


  void Imm::Box(const glm::vec3& p0, const glm::vec3& p1, float ctex)
  {
    Box(p0, p1, defaultColor, ctex);
  }


  void Imm::Box(const glmutils::bbox3& box, const color32& color)
  {
    Box(box.p0(), box.p1(), color, ignoreTexCoord);
  }


  void Imm::Box(const glmutils::bbox3& box, float ctex)
  {
    Box(box.p0(), box.p1(), defaultColor, ctex);
  }


  void Imm::Box(const glm::mat4 M, const glm::vec3& p0, const glm::vec3& p1, const color32& color)
  {
    Box(M, p0, p1, color, ignoreTexCoord);
  }


  void Imm::Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, float ctex)
  {
    Box(M, p0, p1, defaultColor, ctex);
  }


  void Imm::Box(const glm::mat4& M, const glmutils::bbox3& box, const color32& color)
  {
    Box(M, box.p0(), box.p1(), color, ignoreTexCoord);
  }


  void Imm::Box(const glm::mat4& M, const glmutils::bbox3& box, float ctex)
  {
    Box(M, box.p0(), box.p1(), defaultColor, ctex);
  }


  void Imm::Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex)
  {
    using glmutils::transformPoint;

    Begin(IMM_LINE_STRIP);
    Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p1.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p1.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p1.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p1.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p1.z)), color, ctex);
    End();
    Begin(IMM_LINES);
    Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p1.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p1.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p0.z)), color, ctex);
    Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p1.z)), color, ctex);
    End();
  }

}

