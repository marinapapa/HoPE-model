#ifndef PIGEON_HPP_INCLUDED
#define PIGEON_HPP_INCLUDED

#include <istream>
#include <ostream>
#include <torus.hpp>
#include <glmutils/random.hpp>
#include <glm/glm.hpp>
#include "states/transient.hpp"
#include "actions/align_actions.hpp"
#include "actions/cohere_actions.hpp"
#include "actions/cohere_speed_actions.hpp"
#include "actions/avoid_actions.hpp"
#include "actions/predator_actions.hpp"
#include "actions/avoid_pred_actions.hpp"
#include "actions/no_interacting_actions.hpp"
#include "model/flight_control.hpp"
#include "model/flight.hpp"
#include "model/json.hpp"


namespace model {
  
  template <>
  struct known_color_maps<pigeon_tag> 
  {
    static constexpr size_t size = 8;
    static constexpr const char* descr[size] = {
      "none",
	   "idx",
      "speed",
      "banking",
      "state",
      "nnd",
	  "flock",
      "targeted"
    };
  };


  template <>
  struct snapshot_entry<pigeon_tag>
  {
      vec_t pos = vec_t(0);
      vec_t dir = vec_t(0);
      float speed = 0.f;
      vec_t accel = vec_t(0);
  };

  class Pigeon
  {
  public:
    using Tag = pigeon_tag;

    static constexpr const char* name() { return "Pigeon"; }

    using AP = states::package<
      states::transient<actions::package<Pigeon, // normal flocking
        actions::align_n<Pigeon>,
		    actions::cohere_turn_n_all<Pigeon>,
        actions::cohere_accel_n_front<Pigeon>,
        actions::avoid_n_position<Pigeon>,
        actions::wiggle<Pigeon>,
        actions::avoid_p_direction<Pigeon>
        >>
    >;

  public:
    Pigeon(Pigeon&&) = default;
    Pigeon(size_t idx, const json& J);

    void initialize(size_t idx, const Simulation& sim, const json& J);

    // returns next update time
    tick_t update(size_t idx, tick_t T, const Simulation& sim);
    void integrate(tick_t T, const Simulation& sim);
    void on_state_exit(size_t idx, tick_t T, const Simulation& sim);

    ::model::instance_proxy instance_proxy(long long color_map, size_t idx, const class Simulation* sim) const noexcept;
    ::model::snapshot_entry<Tag> snapshot(const Simulation* sim, size_t idx) const noexcept;
    void snapshot(Simulation* sim, size_t idx, const snapshot_entry<Tag>& se) noexcept;
    static float distance2(const pos_t& a, const pos_t& b) { return torus::distance2(Simulation::WH(), a, b); }
    static float bearing_angl(const vec_t& d, const pos_t& a, const pos_t& b) { return math::rad_between(d, torus::ofs(Simulation::WH(), a, b)); }
   
    const int& get_current_state() const noexcept { return current_state_; }

  public:
    // accessible from states:
    pos_t pos;   // [m]
    vec_t dir;
    float speed;  // [m/tick]
    float ang_vel = 0; // [ 1/s ] Only for extracting data, not used in model
    vec_t accel;  // [m/tick ^ 2]
    tick_t reaction_time = 0;   // [ticks]
    tick_t last_update = 0; 
    bool am_target = false; // if individual is a the target of the predator
    std::array<float, AP::size> tm; // transition matrix line per state change evaluation for export
    vec_t force;             // reserved for physical forces  [kg * m/tick^2]
    float f_ali_ang = 0.f;        // angle of alignment force
    float f_coh_ang = 0.f;         // angle of coherence force
    float f_sep_ang = 0.f;        // angle of separation force
    vec_t steering;    // linear, lateral  [kg * m/tick^2]

    flight::aero_info<float> ai;
    flight::state_aero<float> sa;
    static std::vector<snapshot_entry<Tag>> init_pop(const Simulation& sim, const json& J);

  private:
    int current_state_ = 0;
    AP::package_array pa_;
  };

}
#endif
