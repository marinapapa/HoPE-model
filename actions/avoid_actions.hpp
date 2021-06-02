#ifndef AVOID_ACTIONS_HPP_INCLUDED
#define AVOID_ACTIONS_HPP_INCLUDED

#include "action_base.hpp"
#include <glmutils/ray.hpp>
#include <glmutils/random.hpp>
#include "model/while_topo.hpp"

namespace model {
  namespace actions {
    // PREY - PREY INTERACTIONS 

    template <typename Agent>
    class avoid_n_position
    { // avoid neighbors position

      make_action_from_this(avoid_n_position);

    public:
      avoid_n_position() {}
      avoid_n_position(size_t, const json& J)
      {
        topo = J["topo"]; // [1]

        auto fov = J["fov"]; // [deg]
        cfov = glm::cos(glm::radians(180.0f - 0.5f * (360.0f - fov))); // [1]

        float minsep = J["minsep"];       // [m]
        minsep2 = minsep * minsep;        // [m^2]

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

        auto ofss = vec_t(0);
        auto realized_topo = while_topo(sv, topo, [&](const auto& ni) {

          if (in_fov(self, ni.dist2, flock[ni.idx].pos, this))
          {
            if (ni.dist2 < minsep2)
            {
				ofss += torus::ofs(Simulation::WH(), flock[ni.idx].pos, self->pos);
              return true;
            }
          }
          return false;
        });

		const vec_t Fdir = math::save_normalize(ofss, vec_t(0.f)) * w_;
        self->f_sep_ang = math::rad_between(self->dir, Fdir);
		self->steering += Fdir;
      }

       public:
        int topo = 0;           // [1]
        float cfov = 0;         // [1]
        float minsep2 = 0;      // [m^2]
        float maxdist2 = 0;     // [m^2]
    private:
        float w_;               // [1]
      };
  }
}

#endif