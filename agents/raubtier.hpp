#ifndef RAUBTIER_HPP_INCLUDED
#define RAUBTIER_HPP_INCLUDED

#include <json/json.hpp>
#include <torus.hpp>
#include "states/transient.hpp"
#include "states/persistent.hpp"
#include "actions/align_actions.hpp"
#include "actions/cohere_actions.hpp"
#include "actions/avoid_actions.hpp"
#include "actions/no_interacting_actions.hpp"
#include "model/transitions.hpp"
#include "model/flight.hpp"


namespace model {
  
  template <>
  struct known_color_maps<pred_tag>
  {
    static constexpr size_t size = 5;
    static constexpr const char* descr[size] = {
      "none",
      "index",
      "speed",
      "banking",
      "state"
    };
  };


  namespace actions {

    template <typename Agent>
    class do_something
    {
      make_action_from_this(do_something);

    public:
      do_something() {}
      do_something(size_t, const json& J)
      {
        rtau_ = 1.f / J["tau"];    // [1/s]
        speed_ = J["speed"];       // [m/s]
        turn_ = J["turn"];
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        self->steering_.y += turn_;     // constant force turn
        self->cruiseSpeed_ = speed_;
        self->rtau_ = rtau_;
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim) {}

    private:
      float rtau_ = 0;  // [1/s]
      float speed_ = 0; // [m/s]
      float turn_ = 0;
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
        radius_ = J["radius"];
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        // Fz = m * v*v/r
        auto Fz = self->ai.bodyMass * self->speed_ * self->speed_ / radius_;
        self->steering_.y += Fz;
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim) {}

    private:
      float radius_ = 0;          // [m]
    };


    // turn in time
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
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        // Fz = m * v*v/r
        auto Fz = self->ai.bodyMass * self->speed_ * self->speed_ / r_;
        self->steering_.y += Fz;
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim) 
      {
        // we want to turn turn_ radians in time_ seconds.
        auto w = turn_ / time_;       // required angular velocity
        r_ = self->speed_ / w;       // radius
      }

    private:
      float r_ = 0;
      float turn_ = 0;          // rad
      float time_ = 0;
    };

  }


  class Raubtier
  {
  public:
    using Tag = pred_tag;

    static constexpr const char* name() { return "Raubtier"; }

    using AP = states::package<
      states::persistent<actions::package<Raubtier,
        actions::do_something<Raubtier>
      >>,
      states::persistent<actions::package<Raubtier,
        actions::do_something<Raubtier>
      >>,
      states::persistent<actions::package<Raubtier,
        actions::r_turn<Raubtier>
      >>,
      states::persistent<actions::package<Raubtier,
        actions::t_turn<Raubtier>
      >>
    >;
    using transitions = transitions::piecewise_linear_interpolator<AP::transition_matrix, 1>;

  public:
    Raubtier(Raubtier&&) = default;
    Raubtier(size_t idx, const json& J);
    void initialize(size_t idx, const Simulation& sim);

    // returns next update time
    tick_t update(size_t idx, tick_t T, const Simulation& sim);
    void integrate(tick_t T, const Simulation& sim);
    void on_state_exit(size_t idx, tick_t T, const Simulation& sim);

    static float distance2(const pos_t& a, const pos_t& b) {
      return torus::distance2(Simulation::WH(), a, b);
    }
    ::model::instance_proxy instance_proxy(long long color_map, size_t idx, const class Simulation* sim) const noexcept;
    
    // accessible from states
    pos_t pos_;
    vec_t dir_;
    vec_t steering_;    // linear, lateral
    vec_t force_;       // reserved for physical forces
    vec_t accel_;
    tick_t reaction_time_ = 0;
    tick_t last_update = 0;
    float speed_ = 0;
    float cruiseSpeed_ = 0;
    float rtau_ = 0;            // speed control 1/tau [1/s]
    const float minSpeed_ = 0;
    const float maxSpeed_ = 0;
    int current_state_ = 0;
    static const flight::aero_info<float>& ai;

  private:
    // banking angle
    float bank() const;

    static flight::aero_info<float> ai_;
    static transitions transitions_;
    AP::package_array pa_;
  };


}
#endif
