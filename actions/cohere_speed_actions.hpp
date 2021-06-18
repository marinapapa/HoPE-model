#ifndef COHERE_SPEED_ACTIONS_HPP_INCLUDED
#define COHERE_SPEED_ACTIONS_HPP_INCLUDED

#include "model/action_base.hpp"

namespace model {
  namespace actions {

    template <typename Agent>
    class cohere_accel_n_front
    {  // cohere by speed change with neighbors positioned in front

      make_action_from_this(cohere_accel_n_front);

    public:
      cohere_accel_n_front() {}
      cohere_accel_n_front(size_t, const json& J)
      {
		    topo = J["topo"];    // [1]
		    w_ = J["w"];    // [1]
            decel_w_ = J["decel_w"];    // [1]

        auto fov = J["fov"]; // [deg]
        ffov = J["ffov"];    // [deg]
        cfov = glm::cos(glm::radians(180.0f - 0.5f * (360.0f - float(fov)))); // [1]

		    float maxdist = J["maxdist"];     // [m]
		    maxdist2 = maxdist * maxdist;     // [m^2]

		    float minacceldist = J["min_accel_dist"];     // [m]
            minacceldist2 = minacceldist; // *minacceldist;     // [m^2]
		    float maxacceldist = J["max_accel_dist"];     // [m]
            maxacceldist2 = maxacceldist; // *maxacceldist;     // [m^2]
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
      }

	    void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
	    {
		    const auto nv = sim.sorted_view<Tag>(idx);
		    const auto& flock = sim.pop<Tag>();

            auto av_f_dist2 = 0.f; // average distance to neighbors
		    auto realized_topo = while_topo(nv, topo, [&](const auto& ni) {
   		  if (in_fov(self, ni.dist2, flock[ni.idx].pos, this))
			  {
				  if (!(torus::is_atside(self->pos, self->dir, flock[ni.idx].pos, ffov, Simulation::WH())))
				  {
					  av_f_dist2 += ni.dist2;
					  return true;
				  }
                  return false;
			  }
			  return false;
		    });

        auto w = (realized_topo) ?
              math::smootherstep(std::sqrt(av_f_dist2) / realized_topo, minacceldist2, maxacceldist2)
             : - decel_w_;
		self->steering += w_ * w * self->dir;
	  }

    public:
      int topo = 0;         // [1]
      float cfov = 0;       // [1]
      float ffov = 0;       // [deg] front field of view
      float maxdist2 = 0;   // [m^2]
	  float minacceldist2 = 0; // [m^2]
	  float maxacceldist2 = 0; // [m^2]
    private:
      float w_ = 0;         // [1] 
      float decel_w_ = 0;
    };
  }
}

#endif