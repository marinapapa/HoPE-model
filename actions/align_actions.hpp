#ifndef ALIGN_ACTIONS_HPP_INCLUDED
#define ALIGN_ACTIONS_HPP_INCLUDED

#include "model/while_topo.hpp"
#include "model/action_base.hpp"


namespace model {

  namespace actions {

    template <typename Agent>
    class align_n
    { // align by turning with all neighbors

      make_action_from_this(align_n);

    public:
      align_n() {}
      align_n(size_t, const json& J)
      {
        topo = J["topo"];     // [1]
        auto fov = J["fov"];  // [deg]
        cfov = glm::cos(glm::radians(180.0f - 0.5f * (360.0f - float(fov)))); // [1]

        // max distance
        float maxdist = J["maxdist"];    // [m]
        maxdist2 = maxdist * maxdist;    // [m^2]
        w_ = J["w"];                     // [1]
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        const auto sv = sim.sorted_view<Tag>(idx);
        const auto& flock = sim.pop<Tag>();
        
		    vec_t adir(0.f);
        auto realized_topo = while_topo(sv, topo, [&](const auto& ni) {

          if (in_fov(self, ni.dist2, flock[ni.idx].pos, this))
          {
            adir += flock[ni.idx].dir;
            return true;
          }
          return false;
        });

        const vec_t Fdir = math::save_normalize(adir, vec_t(0.f)) * w_; 
        self->f_ali_ang = math::rad_between(self->dir, Fdir);
 		self->steering += Fdir;
      }

    public:
      int topo = 0;       // [1]
      float cfov = 0;     // [1]
      float maxdist2 = 0; // [m^2]

    private:
      float w_;           // [1]
    }; 

  }
}

#endif
