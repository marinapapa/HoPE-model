#ifndef TURTLE_HPP_INCLUDED
#define TURTLE_HPP_INCLUDED

#include <json/json.hpp>
#include <torus.hpp>
#include "states.hpp"
#include "actions.hpp"


namespace model {


  class Prey
  {
  public:
    static constexpr const char* name() { return "Prey"; }

    using AP = states::package<
      states::flocking<actions::package<
        actions::align_with_n<Prey>,
        actions::cohere_with_n<Prey>,
        actions::avoid_n<Prey>,
        actions::wiggle<Prey>
      >>,
      states::turn<actions::empty_package<Prey>>,
      states::turn<actions::package<
        actions::wiggle<Prey>
      >>
    >;

  public:
    Prey(Prey&&) = default;
    Prey(size_t idx, const json& J);
    void initialize(size_t idx, tick_t T, const Simulation& sim);
    
    // returns next update time
    tick_t update(size_t idx, tick_t T, const Simulation& sim);
    void integrate(tick_t T, const Simulation& sim);
    void on_state_exit(size_t idx, tick_t T, const Simulation& sim);

    static float distance2(const pos_t& a, const pos_t& b) {
      return torus::distance2(WH, a, b);
    }
    operator gl_agent_proxy() const noexcept;

    // accessible from states
    pos_t pos;
    vec_t dir, old_dir, new_dir;
    tick_t reaction_time = 0;
    tick_t last_update = 0;
    static float WH;
  private:
    int current_state_ = 0;
    float speed_ = 0;
    static AP::transition_matrix T_;
    AP::package_array pa_;

  };


  class Pred
  {
  public:
    static constexpr const char* name() { return "Pred"; }

    using AP = states::package<
      states::flocking<actions::package<
        actions::wiggle<Pred>
      >>,
      states::turn<actions::empty_package<Pred>>,
      states::turn<actions::empty_package<Pred>>
    >;

  public:
    Pred(Pred&&) = default;
    Pred(size_t idx, const json& J);
    void initialize(size_t idx, tick_t T, const Simulation& sim);

    // returns next update time
    tick_t update(size_t idx, tick_t T, const Simulation& sim);
    void integrate(tick_t T, const Simulation& sim);
    void on_action_exit(size_t idx, tick_t T, const Simulation& sim);

    static float distance2(const pos_t& a, const pos_t& b) {
      return torus::distance2(WH, a, b);
    }
    operator gl_agent_proxy() const noexcept;

    // accessible from states
    pos_t pos;
    vec_t dir, old_dir, new_dir;
    tick_t reaction_time = 0;
    tick_t last_update = 0;

  private:
    int current_state_ = 0;
    float speed_ = 0;
    static AP::transition_matrix T_;
    AP::package_array pa_;
    static float WH;
  };


}


#endif
