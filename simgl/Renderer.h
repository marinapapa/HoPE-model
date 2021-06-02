#ifndef PIGEON_RENDERER_H_INCLUDED
#define PIGEON_RENDERER_H_INCLUDED

#include <memory>
#include <mutex>
#include <glsl/context.h>
#include <glsl/wgl_context.hpp>
#include <json/json.hpp>
#include "agents/agents.hpp"
#include "shader.h"
#include "csDevice.hpp"


struct Param
{
  struct sim_param
  {
    double speedup = 1.0;
    std::array<float, 3> bkgColor = { { 0.0f, 0.5f, 0.5f } };
    std::array<float, 3> tbkgColor = { { 179.0f / 255.f, 180.0f / 255.f, 209.f / 255.f } };
    float bkgDimm = 1.f;
    int textwidth96 = 236;
    int textwidth = 236;           // [px]
    int border = 5;                // [px]
    std::array<bool, model::n_species> trail = { false };
  } sim;
};


struct nearest_instance_record {
  model::instance_proxy* pInstance = nullptr;
  size_t species = 0;
  size_t idx = 0;
  float dist2 = 0.f;
};


class Renderer
{
public:
  Renderer(glsl::Context&& glctx, const json& J, const Param param);
  ~Renderer();


  bool onKeyDown(int key, bool shift, bool alt, bool ctrl);
  void onSize(int W, int H);
  void onMouseClick(int mouseX, int mouseY);
  void onMouseTrack(int deltaX, int deltaY);
  void onMouseWheel(int mouseX, int mouseY, int deltaZ);
  void onDpiChanged(int dpi);
  bool hit_test(int mouseX, int mouseY) const;
  void flush_state(const class AppWin& app, const model::Simulation& sim);
  void set_color_map(unsigned species, unsigned map);

  // nearest instance alive
  nearest_instance_record nearest_instance(int mouseX, int mouseY);
  void follow(size_t species, size_t idx, bool flock);
  void render();

private:
  mutable std::mutex mutex_;
  std::unique_ptr<class RendererImpl> pimpl_;
};


#endif
