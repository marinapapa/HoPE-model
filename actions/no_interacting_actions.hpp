#ifndef OTHER_ACTIONS_HPP_INCLUDED
#define OTHER_ACTIONS_HPP_INCLUDED

#include "model/action_base.hpp"

namespace model {
  namespace actions {
    
    template <typename Agent>
    class wiggle
    {
      make_action_from_this(wiggle);

    public:
      wiggle() {}
      wiggle(size_t, const json& J)
      {
		    w_ = J["w"];               // [deg/s]
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
      }

      void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
      {
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        auto w = std::uniform_real_distribution<float>(-w_, w_)(reng); // [rad]
		    self->steering += glmutils::perpDot(self->dir) * w;
      }

    private:
      float w_ = 0;      // [1] 

    };


    // constant radius turn 
    template <typename Agent>
    class r_turn
    {
      make_action_from_this(r_turn);

    public:
      r_turn() {}
      r_turn(size_t, const json& J)
      {
        turn_ = J["turn"];               // [deg]
        turn_ = glm::radians(turn_);     // [rad]
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        // Fz = m * v*v/r
        auto Fz = self->ai.bodyMass * self->speed * self->speed / turn_;
		    self->steering += Fz * glmutils::perpDot(self->dir);
      }

    private:
      float turn_ = 0; // [rad/s]
      float w_ = 0;      // [1] NOT USED BUT IMPORTANT FOR NORMALIZING ALL WEIGHTS
    };


    //turn in time
    template <typename Agent>
    class t_turn
    {
      make_action_from_this(t_turn);

    public:
      t_turn() {}
      t_turn(size_t, const json& J)
      {
        turn_ = glm::radians(float(J["turn"]));
        time_ = J["time"];
        
        if (time_ == 0.f || turn_ == 0.f) throw std::runtime_error("wrong parameters in t_turn");
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        // we want to turn turn_ radians in time_ seconds.
        auto w = turn_ / time_;       // required angular velocity
        r_ = self->speed / w;       // radius
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        // Fz = m * v*v/r
        auto Fz = self->ai.bodyMass * self->speed * self->speed / r_;
        self->steering += Fz * glmutils::perpDot(self->dir);
      }

    private:
      float r_ = 0;
      float turn_ = 0;   // [rad]
      float time_ = 0;
      float w_ = 0;      // [1] NOT USED BUT IMPORTANT FOR NORMALIZING ALL WEIGHTS
    };

  }
}


#endif