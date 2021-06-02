#define _USE_MATH_DEFINES
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION

#include <cmath>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/perp_dot.hpp>
#include <glsl/stb_image.h>
#include <glsl/ac3d.hpp>
#include <glsl/camera.h>
#include <glsl/debug.h>
#include <glsl/bmfont.hpp>
#include <glsl/imm.hpp>
#include <glsl/text.hpp>
#include <libs/game_watches.hpp>
#include "Renderer.h"
#include "AppWin.h"


namespace filesystem = std::filesystem;
using namespace shader;
using model::instance_proxy;


namespace {

  void DeleteSync(GLsync& sync)
  {
    if (sync) glDeleteSync(sync);
    sync = nullptr;
  }


  filesystem::path GetExeDirectory()
  {
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, sizeof(buf));
    return filesystem::path(buf).parent_path();
  }


  glm::dvec3 cameraScreenDirection(int winX, int winY, int winZ, const glm::dmat4& IVP, const glm::ivec4& viewport)
  {
    auto tmp = glm::dvec4(winX, winY, winZ, 1);
    tmp.x = (tmp.x - viewport[0]) / viewport[2];
    tmp.y = ((viewport[3] - tmp.y) - viewport[1]) / viewport[3];
    tmp = tmp * 2.0 - 1.0;    // point in screen space
    auto obj = IVP * tmp;     // hom. point in world space 
    obj /= obj.w;             // point in world space
    return glm::dvec3(obj);
  }


  GLuint LoadTexture(GLenum texUnit, const filesystem::path& path)
  {
    auto FileName = path.string();
    GLint width, height, channels;
    auto texData = stbi_load(FileName.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (0 == texData)
    {
      throw std::exception((std::string("Can't read texture map ") + FileName).c_str());
    }
    GLuint tex = 0; glGenTextures(1, &tex);
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    stbi_image_free(texData);
    return tex;
  }


  struct gl_trail_config
  {
    gl_trail_config() = default;
    explicit gl_trail_config(const json& J)
    {
      nextUpdate = 0;
      interval = static_cast<model::tick_t>(double(J["Trails"]["interval"]) / double(J["Simulation"]["dt"]));
      length = static_cast<model::tick_t>(double(J["Trails"]["length"]) / double(J["Simulation"]["dt"]));
      fade = J["Trails"]["fade"];
      ////width = J["Trails"]["width"];
      ////tickInterval = J["Trails"]["tickInterval"];
      ////tickWidth = J["Trails"]["tickWidth"];
    }

    model::tick_t nextUpdate;
    model::tick_t interval;
    size_t length;
    float fade;
    float width;
    float tickInterval;
    float tickWidth;
  };


  struct gl_trail
  {
    gl_trail() = default;
    gl_trail(const gl_trail& rhs) = default;
    gl_trail(gl_trail&& rhs) : gl_trail()
    {
      *this = std::move(rhs);
    }

  private:
    gl_trail& operator=(const gl_trail& rhs) = default;

  public:
    gl_trail& operator=(gl_trail&& rhs)
    {
      N = rhs.N; rhs.N = 0;
      size = rhs.size; rhs.size = 0;
      capacity = rhs.capacity; rhs.capacity = 0;
      ssbo = rhs.ssbo; rhs.ssbo = GL_NONE;
      ofs = rhs.ofs; rhs.ofs = 0;
      return *this;
    }

    gl_trail(const char* name, const gl_trail_config& tconfig, const json& J)
    {
      N = J[name]["N"];
      size = 0;
      capacity = static_cast<GLsizei>(tconfig.length / tconfig.interval);
      glGenBuffers(1, &ssbo);
      const auto elem_size = static_cast<GLsizei>(sizeof(instance_proxy));
      const auto row_size = static_cast<GLsizei>(N * sizeof(instance_proxy));
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
      glBufferStorage(GL_SHADER_STORAGE_BUFFER, 2 * (capacity + 1) * row_size, nullptr, 0);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    ~gl_trail() 
    {
      glDeleteBuffers(1, &ssbo);
    }

    void push_back(GLuint dst_vbo)
    {
      const auto row_size = static_cast<GLsizei>(N * sizeof(instance_proxy));
      glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo);
      glBindBuffer(GL_COPY_READ_BUFFER, ssbo);
      const auto writeofs = ofs ? 0 : capacity;
      glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        ofs * row_size,
        (writeofs + 1) * row_size,
        (capacity - 1) * row_size
      );
      glBindBuffer(GL_COPY_READ_BUFFER, dst_vbo);
      glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        0,
        writeofs * row_size,
        row_size
      );
      size = std::min(size + 1, capacity);
      ofs = writeofs;
    }

    void render() const 
    {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
      glUniform1i(0, GLint(size));
      glUniform1i(1, GLint(N));
      glUniform1i(2, GLint(N * ofs));
      glDrawArraysInstanced(GL_POINTS, 0, size, N);
//      glDrawElementsInstanced(GL_POINTS, capacity, GL_UNSIGNED_INT, (void*)(0), N);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    GLsizei N = 0;
    GLsizei size = 0;
    GLsizei capacity = 0;
    GLuint ssbo = GL_NONE;
    GLsizei ofs = 0;
  };


  struct gl_species
  {
    gl_species(const gl_species&) = delete;
    gl_species(gl_species&&);
    gl_species& operator=(gl_species&&);

    gl_species() = default;
    gl_species(const std::filesystem::path& binPath, const gl_trail_config& tconfig, const char* name, const json& J);
    ~gl_species();
    void render_trail() const;
    void render() const;

    GLsizei size = 0;
    GLuint tex = GL_NONE;
    float scale = 1.f;
    long long color_map = 0;
    GLuint vao = GL_NONE;
    GLuint vbo_vert = GL_NONE;
    GLuint vbo_idx = GL_NONE;
    GLuint vbo_inst = GL_NONE;
    model::instance_proxy* pInstance = nullptr;
    gl_trail trail;
    glsl::ac3d_model ac;
  };


  gl_species::gl_species(gl_species&& rhs) :
    gl_species()
  {
    *this = std::move(rhs);
  }


  gl_species& gl_species::operator=(gl_species&& rhs)
  {
    this->~gl_species();
    size = rhs.size; rhs.size = 0;
    tex = rhs.tex; rhs.tex = GL_NONE;
    scale = rhs.scale;
    vao = rhs.vao; rhs.vao = GL_NONE;
    vbo_vert = rhs.vbo_vert; rhs.vbo_vert = GL_NONE;
    vbo_idx = rhs.vbo_idx; rhs.vbo_idx = GL_NONE;
    vbo_inst = rhs.vbo_inst; rhs.vbo_inst = GL_NONE;
    pInstance = rhs.pInstance; rhs.pInstance = nullptr;
    trail = std::move(rhs.trail);
    ac = std::move(rhs.ac);
    return *this;
  }


  gl_species::gl_species(const std::filesystem::path& binPath, const gl_trail_config& tconfig, const char* name, const json& J) :
    trail(name, tconfig, J)
  {
    size = J[name]["N"];
    scale = float(J[name]["scale"]) * float(J[name]["aero"]["wingSpan"]);
    ac = glsl::ImportAC3D((binPath / "media" / std::string(J[name]["shape"])).string());
    tex = LoadTexture(GL_TEXTURE0 + 1, binPath / "media" / ac.texFile);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo_vert);
    glGenBuffers(1, &vbo_idx);
    glGenBuffers(1, &vbo_inst);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_idx);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * ac.indices.size(), ac.indices.data(), GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vert);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(glsl::V4F_N3F_T2S) * ac.vertices.size(), ac.vertices.data(), GL_NONE);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glsl::V4F_N3F_T2S), (void*)0);
    glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(glsl::V4F_N3F_T2S), (void*)(offsetof(struct glsl::V4F_N3F_T2S, t)));

    const auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_inst);
    glBufferStorage(GL_ARRAY_BUFFER, size * sizeof(instance_proxy), nullptr, flags);
    pInstance = (instance_proxy*)glMapBufferRange(GL_ARRAY_BUFFER, 0, size * sizeof(instance_proxy), flags);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(instance_proxy), (void*)0);                      // pos_vel
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(instance_proxy), (void*)(sizeof(glm::vec4)));    // side_tex_alpha
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
  }


  gl_species::~gl_species()
  {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo_vert);
    glDeleteBuffers(1, &vbo_idx);
    glDeleteBuffers(1, &vbo_inst);
    glDeleteTextures(1, &tex);
    pInstance = nullptr;        // debug
  }


  void gl_species::render() const
  {
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1f(0, scale);
    glDrawElementsInstanced(GL_TRIANGLES,
      static_cast<GLsizei>(ac.indices.size()),
      GL_UNSIGNED_INT, (GLvoid*)(0),
      size);
    glBindVertexArray(0);
  }


  void gl_species::render_trail() const
  {
    trail.render();
  }


  static constexpr size_t n_species = std::tuple_size_v<model::species_pop>;
  using gl_species_array = std::array<gl_species, n_species>;


  template <size_t I>
  void init_species(const std::filesystem::path& binPath, const gl_trail_config& tconfig, const json& J, gl_species_array& out)
  {
    using agent_type = typename std::tuple_element_t<I, model::species_pop>::value_type;
    out[I] = std::move(gl_species(binPath, tconfig, agent_type::name(), J));
    init_species<I + 1>(binPath, tconfig, J, out);
  }

  template <>
  void init_species<n_species>(const std::filesystem::path&, const gl_trail_config& tconfig, const json&, gl_species_array&)
  {}

}


class RendererImpl
{
public:
  RendererImpl(glsl::Context&& glctx, const json& J, const Param param);
  ~RendererImpl();

  bool onKeyDown(int key, bool shift, bool alt, bool ctrl);
  void onSize(int W, int H);
  void onMouseClick(int mouseX, int mouseY);
  void onMouseTrack(int deltaX, int deltaY);
  void onMouseWheel(int mouseX, int mouseY, int deltaZ);
  void onDpiChanged(int dpi);

  bool hit_test(int mouseX, int mouseY) const;
  glm::vec2 area_coor(int x, int y) const;

  void flush_state(const class AppWin& app, const model::Simulation& sim);
  void set_color_map(unsigned species, unsigned map) {
    if (species < model::n_species) {
      gl_species_[species].color_map = map;
    }
  }

  nearest_instance_record nearest_instance(int mouseX, int mouseY);

  void follow(size_t species, size_t idx, bool flock) {
    follow_.idx = -1;
    follow_.species = 0;
    follow_.flock = false;
    if (species < model::n_species) {
      follow_.idx = static_cast<GLsizei>(idx);
      follow_.species = species;
      follow_.flock = flock;
    }
  }

  void cs_render();

  struct follow_t {
    GLsizei idx = -1;
    size_t species = 0;
    bool flock = false;
    model::vec_t eye;
  };
  follow_t& follow() { return follow_; }

private:
  void cs_init_glsl(const json& J);
  void cs_set_viewport();
  void flush_text();
  void flush_camera();
  bool hit_test(int x, int y, const glm::dvec4& vp) const;
  glm::vec2 viewport_coor(int x, int y, const glm::dvec4& vp) const;

  void translate(const glm::vec2& shift);

  enum GLVBO {
    VBO_UNIFORM,    // uniform buffer block: shader::uniformBlock_t
    VBO_ENV,        // background
    VBO_MAX
  };

  enum GLVAO {
    VAO_ENV,        // background
    VAO_MAX
  };

  glsl::Context glctx_;
  std::array<GLuint, VAO_MAX> vao_;
  std::array<GLuint, VBO_MAX> vbo_;
  GLuint spectrumTex_ = 0;
  GLuint envTex_ = 0;
  const float dt_;
  model::tick_t tick_;
  float WH_;
  int dpi_ = USER_DEFAULT_SCREEN_DPI;

  void* pUniBlk_ = nullptr;

  GLuint instanceProg_ = GL_NONE;
  GLuint texProg_ = GL_NONE;
  GLuint immProg_ = GL_NONE;
  GLuint textProg_ = GL_NONE;
  GLuint ribbonProg_ = GL_NONE;
  GLsync flush_sync_ = nullptr;
  GLsync render_sync_ = nullptr;

  // species
  gl_species_array gl_species_;
  gl_trail_config trail_config_;

  game_watches::stop_watch<> fps_watch_;
  std::unique_ptr<glsl::Camera> camera_;
  follow_t follow_;
  float zoom_;

  std::unique_ptr<glsl::Imm> imm_;
  std::unique_ptr<glsl::Text> text_;

  // window stuff
  glm::ivec2 winExt_;
  bool viewport_dirty_ = false;
  Param param_;

  // Font map
  glsl::Text::font_map_t Faces_;
};


RendererImpl::RendererImpl(glsl::Context&& glctx, const json& J, const Param param) : 
  glctx_(std::move(glctx)), 
  winExt_(0), 
  vao_{ 0 }, 
  vbo_{ 0 }, 
  dt_(J["Simulation"]["dt"]),
  WH_(J["Simulation"]["WH"]),
  zoom_(1),
  param_(param)
{
  cs_init_glsl(J);
  camera_.reset(new glsl::Camera());
  fps_watch_.start();
}


RendererImpl::~RendererImpl()
{
  glUseProgram(0);
  glDeleteVertexArrays(static_cast<GLsizei>(vao_.size()), vao_.data());
  glDeleteBuffers(static_cast<GLsizei>(vbo_.size()), vbo_.data());
  glDeleteTextures(1, &spectrumTex_);
  glDeleteTextures(1, &envTex_);
  glDeleteProgram(instanceProg_);
  glDeleteProgram(texProg_);
  glDeleteProgram(immProg_);
  glDeleteProgram(textProg_);
  glDeleteProgram(ribbonProg_);
  DeleteSync(flush_sync_);
  DeleteSync(render_sync_);
  Faces_.clear();
}


bool RendererImpl::hit_test(int x, int y, const glm::dvec4& vp) const
{
  return (x >= int(vp[0]) && y >= int(vp[1]) && x <= int(vp[0] + vp[2]) && y <= int(vp[1] + vp[3]));
}


glm::vec2 RendererImpl::viewport_coor(int x, int y, const glm::dvec4& vp) const
{
  const float x_ = float(x) - float(vp[0]);
  const float y_ = float(winExt_.y - y) - float(vp[1]);
  return glm::vec2(2.f * x_ / float(vp[2]) - 1.f, 2.f * y_ / float(vp[3]) - 1.f);
}


glm::vec2 RendererImpl::area_coor(int x, int y) const
{
  const auto MVP = glm::mat4(camera_->P() * camera_->V()) * glm::mat4(zoom_);
  const auto IMVP = glm::inverse(MVP);
  const auto vp_coor = glm::vec4(viewport_coor(x, y, camera_->viewport()), 0.f, 1.f);
  const auto tc = IMVP * vp_coor * glm::vec4(zoom_);
  return glm::vec2(tc);

}


void RendererImpl::translate(const glm::vec2& shift)
{
  const auto V = camera_->V();
  camera_->setViewMatrix(glm::translate(V, glm::dvec3(shift.x, shift.y, 0.)));
}


bool RendererImpl::onKeyDown(int key, bool shift, bool alt, bool ctrl)
{
  return false;
}


void RendererImpl::onSize(int W, int H)
{
  winExt_ = glm::ivec2(W, H);
  viewport_dirty_ = true;
}


void RendererImpl::onMouseClick(int mouseX, int mouseY)
{
}


void RendererImpl::onMouseTrack(int deltaX, int deltaY)
{
  translate((area_coor(0, 0) - area_coor(deltaX, deltaY)));
  viewport_dirty_ = true;
}


void RendererImpl::onMouseWheel(int mouseX, int mouseY, int deltaZ)
{
  const auto vp = camera_->viewport();
  if (hit_test(mouseX, mouseY, vp)) {
    zoom_ = std::max(1.0f, zoom_ + 0.25f * deltaZ);
    viewport_dirty_ = true;
  }
}


void RendererImpl::onDpiChanged(int dpi)
{
  dpi_ = dpi;
  auto dpiScale = double(dpi_) / USER_DEFAULT_SCREEN_DPI;
  param_.sim.textwidth = static_cast<int>(dpiScale * param_.sim.textwidth96);
}


bool RendererImpl::hit_test(int mouseX, int mouseY) const
{
  return hit_test(mouseX, mouseY, camera_->viewport());
}


// nearest instance alive
nearest_instance_record RendererImpl::nearest_instance(int mouseX, int mouseY)
{
  auto nnr = nearest_instance_record();
  if (hit_test(mouseX, mouseY)) {
    auto sim_pos = torus::wrap(model::Simulation::WH(), area_coor(mouseX, mouseY));
    auto mindd = std::numeric_limits<float>::max();
    for (size_t s = 0; s < model::n_species; ++s) {
      auto pi = gl_species_[s].pInstance;
      if (pi->alpha != 0.f) {
        for (size_t i = 0; i < gl_species_[s].size; ++i, ++pi) {
          const auto dd = torus::distance2(model::Simulation::WH(), sim_pos, pi->pos);
          if (dd < mindd) {
            nnr = { pi, s, i, dd };
            mindd = dd;
          }
        }
      }
    }
  }
  return nnr;
}


void RendererImpl::cs_set_viewport()
{
  const auto& sim = param_.sim;
  glm::ivec4 vp(0, 0, winExt_.x - sim.textwidth, winExt_.y);
  const auto world = glm::dvec4(-WH_ / 2, WH_ / 2, -WH_ / 2, WH_ / 2);
  const auto b = sim.border;
  vp = (vp[2] > vp[3])
    ? glm::ivec4(b, vp[1] + b, vp[3] - 2 * b, vp[3] - 2 * b)
    : glm::ivec4(vp.x + b, b + ((vp[3] - vp[2]) >> 1), vp[2] - 2 * b, vp[2] - 2 * b);
  camera_->setOrthoViewport(vp, world / static_cast<double>(zoom_));

  // wrap
  const auto eye = camera_->eye();
  if (abs(eye.x) > WH_ / 2)
    translate(glm::vec2(copysign(WH_, eye.x), 0));
  if (abs(eye.y) > WH_ / 2)
    translate(glm::vec2(0, copysign(WH_, eye.y)));

  vp = glm::ivec4(camera_->viewport());
  const glm::mat4 MV = glm::mat4(camera_->V());
  auto uniformBlk = uniformBlock_t{
    glm::mat4(camera_->V()),
    glm::mat4(camera_->P()),
    glm::mat4(MV),
    glm::mat4(camera_->P()) * MV,
    glm::mat4(glm::transpose(glm::inverse(MV))),
    glm::mat4(glm::ortho(0.f, float(winExt_.x), float(winExt_.y), 0.f))
  };
  // 0.5: font down-sampling
  const float dpiScale = float(dpi_) / USER_DEFAULT_SCREEN_DPI;
  uniformBlk.Text = glm::scale(uniformBlk.Ortho, glm::vec3(0.5f * dpiScale));
  uniformBlk.zoom = zoom_;
  const auto pUniBlk = static_cast<uniformBlock_t * __restrict>(pUniBlk_);
  *pUniBlk = uniformBlk;
  const float tf[] = {
    0, WH_, 0, 1,
    0, 0, 0, 1,
    WH_, WH_, 0, 1,
    WH_, 0, 0, 1
  };
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[VBO_ENV]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(GLfloat), (GLvoid*)&tf[0]);
  glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}


void RendererImpl::flush_text()
{
  // text
  double fst{};
  const double simTime = static_cast<double>(tick_)* dt_;
  const auto ffract = std::modf(simTime, &fst);
  const auto st = static_cast<int>(fst);
  auto lg2Speedup = static_cast<int>(std::log2(param_.sim.speedup));
  const char* speedupPrefix = (lg2Speedup < 0) ? "1/" : "";
  const int speedupPostfix = static_cast<int>((lg2Speedup < 0) ? 1.0/param_.sim.speedup : param_.sim.speedup);
  char buf[1024];
  auto len = snprintf(&buf[0], sizeof(buf),
    "\\normalb{}Simulation Time  %d:%.2d:%.2d.%.1d\n\n\\white{} \\normal{}speedup: %s%d",
    st / 3600, (st % 3600) / 60, (st % 3600) % 60, static_cast<int>(ffract * 10), speedupPrefix, speedupPostfix);
  text_->set_cursor(glm::ivec2(10, 25));
  text_->print({ buf, buf + len });
}


namespace {


  template <size_t I>
  void flush_species(RendererImpl* self, const model::Simulation& sim, gl_species_array& gls)
  {
    using Tag = std::integral_constant<size_t, I>;
    static_assert(std::is_trivially_destructible_v<model::instance_proxy>);
    auto p_inst = gls[I].pInstance;
    gls[I].size = static_cast<GLsizei>(sim.visit_all<std::integral_constant<size_t, I>>([psim = &sim, p = p_inst, cm = gls[I].color_map](const auto& ind, size_t idx, bool alive) mutable {
      *p = ind.instance_proxy(cm, idx, psim);
      p->alpha = alive ? 1.f : 0.f;
      ++p;
    }));
    auto& follow = self->follow();
    if (follow.species == I && follow.idx >= 0) {
      if (follow.flock) {
        auto flockId = sim.flock_of<Tag>(follow.idx);
        follow.eye = sim.flock_info<Tag>(flockId).gc();
      }
      else {
        follow.eye = sim.pop<Tag>()[follow.idx].pos;
      }
    }
    flush_species<I + 1>(self, sim, gls);
  }

  template <>
  void flush_species<model::n_species>(RendererImpl* self, const model::Simulation&, gl_species_array&)
  {}

}


void RendererImpl::flush_state(const AppWin& app, const model::Simulation& sim)
{
  cs::WaitForSync(render_sync_);
  DeleteSync(render_sync_);
  tick_ = sim.tick();
  this->param_.sim = app.sim_param();
  flush_species<0>(this, sim, gl_species_);
  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
  if (trail_config_.nextUpdate <= tick_) {
    trail_config_.nextUpdate += trail_config_.interval;
    for (auto& sp : gl_species_) sp.trail.push_back(sp.vbo_inst);
  }
  flush_sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}


void RendererImpl::flush_camera()
{
  if (follow_.idx >= 0) {
    const auto eye = follow_.eye; 
    const auto shift = torus::wrap(model::Simulation::WH(), glm::vec2(camera_->eye())) - eye;
    auto M = glm::translate(camera_->V(), glm::dvec3(shift.x, shift.y, 0.));
    camera_->setViewMatrix(M);
    viewport_dirty_ = true;
  }
  if (viewport_dirty_) {
    cs_set_viewport();
    viewport_dirty_ = false;
  }
}


void RendererImpl::cs_render()
{
  cs::WaitForSync(flush_sync_);
  DeleteSync(flush_sync_);
  if (fps_watch_.elapsed<std::chrono::microseconds>() < std::chrono::microseconds(5000)) {
    return;
  }
  fps_watch_.restart();
  flush_camera();
  flush_text();
  text_->Flush();
  imm_->Flush();
  const auto& sim = param_.sim;
  auto bkgColor = sim.bkgColor.data();
  auto vp = glm::ivec4(camera_->viewport());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(bkgColor[0], bkgColor[1], bkgColor[2], 1.0f);
  glViewport(vp.x, vp.y, vp.z, vp.w);
  glBindVertexArray(vao_[VAO_ENV]);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, envTex_);
  glUseProgram(texProg_);
  glUniform1f(0, param_.sim.bkgDimm);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glEnable(GL_BLEND);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, spectrumTex_);
  glUseProgram(ribbonProg_);
  for (size_t i = 0; i < model::n_species; ++i) {
    if (sim.trail[i]) gl_species_[i].render_trail();
  }
  glUseProgram(instanceProg_);
  for (const auto& sp : gl_species_) sp.render();

  glViewport(vp[2] + 2 * sim.border, -sim.border, winExt_.x, winExt_.y);
  text_->Render();
  render_sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  glctx_.SwapBuffers();
}


void RendererImpl::cs_init_glsl(const json& J)
{
  glctx_.MakeCurrent();
  glctx_.SwapInterval(0);
#ifdef GLSL_DEBUG_OUTPUT
  #ifdef GL_DEBUG_OUTPUT
    glEnable(GL_DEBUG_OUTPUT);
  #endif
  glsl::SetDebugCallback(glsl::GLSL_DEBUG_MSG_LEVEL::NOTIFICATION);
#endif
  auto binPath = GetExeDirectory().parent_path();

  glGenVertexArrays(static_cast<GLsizei>(vao_.size()), vao_.data());
  glGenBuffers(static_cast<GLsizei>(vbo_.size()), vbo_.data());
  const auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
  
    // Uniform buffer block
  glBindBuffer(GL_UNIFORM_BUFFER, vbo_[VBO_UNIFORM]);
  glBufferStorage(GL_UNIFORM_BUFFER, sizeof(uniformBlock_t), nullptr, flags); 
  pUniBlk_ = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniformBlock_t), flags);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, vbo_[VBO_UNIFORM]);

  // environment
  envTex_ = LoadTexture(GL_TEXTURE0 + 1, binPath / "media" / std::string(J["Simulation"]["terrain"]));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  texProg_ = ProgFromLiterals(texVertex, texFragment, texGeo);
  glUseProgram(texProg_);
  glBindVertexArray(vao_[VAO_ENV]);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[VBO_ENV]);
  glBufferStorage(GL_ARRAY_BUFFER, 16 * sizeof(GLfloat), nullptr, GL_DYNAMIC_STORAGE_BIT);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // Immediate prog
  imm_.reset(new glsl::Imm());

  // Species
  trail_config_ = gl_trail_config(J);
  instanceProg_ = ProgFromLiterals(birdVertex, birdFragment, birdGeo);
  init_species<0>(binPath, trail_config_, J, gl_species_);
  ribbonProg_ = ProgFromLiterals(ribbonVertex, ribbonFragment, ribbonGeo);
  glUseProgram(ribbonProg_);
  glUniform1f(3, trail_config_.fade / gl_species_[0].trail.capacity);

  // Text
  Faces_["sub"] = bmf::GlyphBuffer::Create((binPath / "media" / "fonts" / "Verdana20.fnt").string().c_str());
  Faces_["small"] = bmf::GlyphBuffer::Create((binPath / "media" / "fonts" / "Verdana24.fnt").string().c_str());
  Faces_["smallb"] = bmf::GlyphBuffer::Create((binPath / "media" / "fonts" / "Verdana24Bold.fnt").string().c_str());
  Faces_["normal"] = bmf::GlyphBuffer::Create((binPath / "media" / "fonts" / "Verdana32.fnt").string().c_str());
  Faces_["normalb"] = bmf::GlyphBuffer::Create((binPath / "media" / "fonts" / "Verdana32Bold.fnt").string().c_str());
  textProg_ = ProgFromLiterals(textVertex, textFragment, textGeo);
  text_.reset(new glsl::Text(textProg_, &Faces_));

  // other stuff
  spectrumTex_ = LoadTexture(GL_TEXTURE0 + 2, binPath / "media" / "spectrum.png");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glDisable(GL_DEPTH_TEST);   // 2D
  glEnable(GL_PROGRAM_POINT_SIZE);
}


#define RLG std::lock_guard<std::mutex> _(mutex_)


Renderer::Renderer(glsl::Context&& glctx, const json& J, const Param param) :
  pimpl_(new RendererImpl(std::move(glctx), J, param))
{
}

Renderer::~Renderer() {};

bool Renderer::onKeyDown(int key, bool shift, bool alt, bool ctrl) { RLG; return pimpl_->onKeyDown(key, shift, alt, ctrl); }
void Renderer::onSize(int W, int H) { RLG; pimpl_->onSize(W, H); }
void Renderer::onMouseClick(int mouseX, int mouseY) { RLG; pimpl_->onMouseClick(mouseX, mouseY); };
void Renderer::onMouseTrack(int deltaX, int deltaY) { RLG; pimpl_->onMouseTrack(deltaX, deltaY); };
void Renderer::onMouseWheel(int mouseX, int mouseY, int deltaZ) { RLG; pimpl_->onMouseWheel(mouseX, mouseY, deltaZ); };
void Renderer::onDpiChanged(int dpi) { RLG;  pimpl_->onDpiChanged(dpi); };
bool Renderer::hit_test(int mouseX, int mouseY) const { RLG; return pimpl_->hit_test(mouseX, mouseY); }
void Renderer::set_color_map(unsigned species, unsigned map) { RLG; pimpl_->set_color_map(species, map); }
nearest_instance_record Renderer::nearest_instance(int mouseX, int mouseY) { RLG; return pimpl_->nearest_instance(mouseX, mouseY); }
void Renderer::follow(size_t species, size_t idx, bool flock) { RLG;  pimpl_->follow(species, idx, flock); }
void Renderer::flush_state(const class AppWin& app, const model::Simulation& sim) { RLG; pimpl_->flush_state(app, sim); }
void Renderer::render() { RLG; pimpl_->cs_render(); }


#undef RLG
