#ifndef MODEL_STATE_TRANSIENT_HPP_INCLUDED
#define MODEL_STATE_TRANSIENT_HPP_INCLUDED

#include "model/state_base.hpp"


namespace model {
  namespace states {


    template <typename IP>
    class transient : public state<typename IP::agent_type>
    {
      make_state_from_this(transient);

    public:
      explicit transient(size_t idx, const json& J) :
        actions(IP::create(idx, J["actions"]))
	    {
	    	sai_ = flight::create_state_aero<float>(J["aeroState"]);
        tr_ = std::max(tick_t(1), static_cast<tick_t>(double(J["tr"]) / Simulation::dt())); // [tick]
        //normalize_actions<0>();
      }

      void enter(agent_type* self, size_t idx, tick_t T, const Simulation& sim) override
      {
        if (tr_ < 1) throw std::runtime_error("Reaction time smaller than 1");
        chain_on_entry<0>(self, idx, T, sim);
      }

      void resume(agent_type* self, size_t idx, size_t T, const Simulation& sim) override
      {
		    self->reaction_time = tr_;
        self->sa = sai_;
        self->sa.cruiseSpeed += self->ai.cruiseSpeedSd;

        chain_actions<0>(self, idx, T, sim);
        self->on_state_exit(idx, T, sim);
      };

    protected: 
      tick_t tr_;  // [tick]
	  flight::state_aero<float> sai_; // state specific aero info
	};

  } 
}

#endif
