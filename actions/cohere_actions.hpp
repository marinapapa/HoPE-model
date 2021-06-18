#ifndef COHERE_TURN_ACTIONS_HPP_INCLUDED
#define COHERE_TURN_ACTIONS_HPP_INCLUDED

#include "model/action_base.hpp"

namespace model {
  namespace actions {

    template <typename Agent>
    class cohere_turn_n_all
    { // cohere by turning with all neighbors

      make_action_from_this(cohere_turn_n_all);

    public:
      cohere_turn_n_all() {}
      cohere_turn_n_all(size_t, const json& J)
      {
        topo = J["topo"];    // [1]

        auto fov = J["fov"]; // [deg]
        cfov = glm::cos(glm::radians(180.0f - 0.5f * (360.0f - float(fov)))); // [1]

        float maxdist = J["maxdist"];     // [m]
        maxdist2 = maxdist * maxdist;     // [m^2]

        w_ = J["w"];                       // [1]
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        const auto sv = sim.sorted_view<Tag>(idx);
        const auto& flock = sim.pop<Tag>();

        auto ofss = vec_t(0.f);
        auto realized_topo = while_topo(sv, topo, [&](const auto& ni) {

          if (in_fov(self, ni.dist2, flock[ni.idx].pos, this))
          {
		       	ofss += torus::ofs(Simulation::WH(), self->pos, flock[ni.idx].pos);
            return true;
          }
          return false;
        });

		      const auto Fdir = math::save_normalize(ofss, vec_t(0.f)) * w_;
              self->f_coh_ang = math::rad_between(self->dir, Fdir);
		      self->steering += Fdir;
      }

    public:
      int topo = 0;           // [1]
      float cfov = 0;         // [1]
      float maxdist2 = 0;     // [m^2]
    
    private:
      float w_ = 0;           // [1]
    };
  }
}

#endif