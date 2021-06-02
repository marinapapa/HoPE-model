#ifndef PRED_HPP_INCLUDED
#define PRED_HPP_INCLUDED

#include <json/json.hpp>
#include <torus.hpp>
#include "states/transient.hpp"
#include "states/persistent.hpp"
#include "actions/align_actions.hpp"
#include "actions/cohere_actions.hpp"
#include "actions/avoid_actions.hpp"
#include "actions/no_interacting_actions.hpp"
#include "actions/hunt_actions.hpp"
#include "actions/predator_actions.hpp"
#include "model/transitions.hpp"
#include "model/flight_control.hpp"
#include "model/flight.hpp"


namespace model {
  
  template <>
  struct known_color_maps<pred_tag>
  {
    static constexpr size_t size = 4;
    static constexpr const char* descr[] = {
      "none",
      "speed",
      "state",
      "banking"
    };
  };


  template <>
  struct snapshot_entry<pred_tag>
  {
    vec_t pos = vec_t(0);
    vec_t dir = vec_t(0);
    float speed = 0.f;
    vel_t accel = vec_t(0);
    bool alive = true;

    static std::istream& stream_from_csv(std::istream& is, snapshot_entry<pred_tag>& e)
    {
      char delim;
      float discard;
      is >> discard >> delim; // discard id in local variable
      is >> e.pos.x >> delim >> e.pos.y >> delim;
      is >> e.dir.x >> delim >> e.dir.y >> delim;
      is >> e.speed >> delim >> e.accel.x >> delim;
      is >> e.accel.y >> delim >> e.alive;
      return is;
    }

    static std::ostream& stream_to_csv(std::ostream& os, const snapshot_entry<pred_tag>& e)
    {
      char delim[] = ", ";
      os << e.pos.x << delim << e.pos.y << delim;
      os << e.dir.x << delim << e.dir.y << delim << e.speed << delim;
      os << e.accel.x << delim << e.accel.y << delim;
      os << e.alive;
      return os;
    }

  };



  class Pred
  {
  public:
    using Tag = pred_tag;

    static constexpr const char* name() { return "Pred"; }

    using AP = states::package<
      states::persistent<actions::package<Pred,
        actions::wiggle<Pred>,
        actions::avoid_closest_prey<Pred>,
        actions::hold_current<Pred>
        >>,
      states::persistent<actions::package<Pred,
        actions::select_flock<Pred>,
        actions::shadowing<Pred>
      >>,
      states::persistent<actions::package<Pred,
        actions::wiggle<Pred>,
        actions::chase_closest_prey<Pred>
      >>,
      states::transient<actions::package<Pred,
        actions::set_retreat<Pred>
        >>
      >;
    using transitions = transitions::piecewise_linear_interpolator<AP::transition_matrix, 1>;

  public:
    Pred(Pred&&) = default;
    Pred(size_t idx, const json& J);
    void initialize(size_t idx, const Simulation& sim, const json& J);

    // returns next update time
    tick_t update(size_t idx, tick_t T, const Simulation& sim);
    void integrate(tick_t T, const Simulation& sim);
    void on_state_exit(size_t idx, tick_t T, const Simulation& sim);

    static float distance2(const pos_t& a, const pos_t& b) {
      return torus::distance2(Simulation::WH(), a, b);
    }
    static float bearing_angl(const vec_t& d, const pos_t& a, const pos_t& b) {
      return math::rad_between(d, torus::ofs(Simulation::WH(), a, b));
    }

    ::model::instance_proxy instance_proxy(long long color_map, size_t idx, const class Simulation* sim) const noexcept;
    ::model::snapshot_entry<Tag> snapshot(const Simulation* sim, size_t idx) const noexcept;
    void snapshot(Simulation* sim, size_t idx, const snapshot_entry<Tag>& se) noexcept;
    static std::vector<snapshot_entry<Tag>> init_pop(const Simulation& sim, const json& J);

    const int& get_current_state() const noexcept { return current_state_; }

  public:
    // accessible from states
    pos_t pos;
    vec_t dir;
    float ang_vel = 0; // [ 1/s ] Only for extracting data, not used in model
    tick_t reaction_time = 0;   // [ticks]
    tick_t last_update = 0;
    float speed = 0.f;            // [m/tick]
    vec_t accel;  // [m/tick ^ 2]
    vec_t force;             // reserved for physical forces  [kg * m/tick^2]
    vec_t steering;    // linear, lateral  [kg * m/tick^2]
    int target_f; // flock target
    int target_i; // individual target
    
    flight::aero_info<float> ai;
	  flight::state_aero<float> sa;

  private:
    int current_state_ = 0;
    static transitions transitions_;
    AP::package_array pa_;
  };


}
#endif
