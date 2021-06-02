#include <queue>
#include <algorithm>
#include <libs/torus.hpp>
#include <libs/graph.hpp>
#include <glmutils/oobb.hpp>
#include "flock.hpp"
#include "simulation.hpp"

namespace model {

  void flock_tracker::cluster(float dd)
  {
    flock_id_.assign(proxy_.size(), no_flock);
    auto last = std::partition(proxy_.begin(), proxy_.end(), [](const auto& ipv) { return ipv.idx != static_cast<unsigned>(-1); });
    proxy_.erase(last, proxy_.end());
    const auto n = proxy_.size();
    auto cc = graph::connected_components(0, static_cast<int>(n), [&](int i, int j) {
      return dd > torus::distance2(Simulation::WH(), proxy_[i].pos, proxy_[j].pos);
    });
    descr_.clear();
    std::vector<vec_t> vpos;
    for (unsigned ci = 0; ci < static_cast<unsigned>(cc.size()); ++ci) {
      vpos.clear();
      vec_t vel = vec_t(0);
      for (auto i : cc[ci]) {
        flock_id_[proxy_[i].idx] = ci;
        vpos.emplace_back(torus::ofs(Simulation::WH(), proxy_[cc[ci][0]].pos, proxy_[i].pos));
        vel += proxy_[i].vel;
      }
      vec_t ext;
      auto H = glmutils::oobb(static_cast<int>(cc[ci].size()), vpos.begin(), ext);
      vel /= cc[ci].size();
      //auto dir = glm::normalize(vel);
      H[2] = glm::vec3(torus::wrap(Simulation::WH(), vec_t(H[2]) + proxy_[cc[ci][0]].pos), 1.f);
      descr_.push_back({ vpos.size(), vel, H, ext });
      int x = 0;
    }
  }


  void flock_tracker::track()
  {
    const auto dt = Simulation::dt();
    for (auto& fd : descr_) {
      vec_t gc = fd.gc();
      fd.H[2] = glm::vec3(torus::wrap(Simulation::WH(), gc + dt * fd.vel), 1.f);
    }
  }

}
