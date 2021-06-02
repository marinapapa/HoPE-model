#include "turtle.hpp"
#include <math.hpp>


namespace model {

  thread_local rndutils::mutable_discrete_distribution<int, rndutils::all_zero_policy_uni> agent_discrete_dist;


  float Prey::WH;
  Prey::AP::transition_matrix Prey::T_;


  Prey::Prey(size_t idx, const json& J) :
    current_state_(0),
    speed_(J["speed"]),
    pos(0, 0),
    dir(1, 0),
    new_dir(1, 0),
    old_dir(1, 0)
  {
    if (idx == 0) {
      WH = J["WH"];
      T_ = J["T"];
    }
    pa_ = AP::create(idx, J["states"]);
  }

  void Prey::initialize(size_t idx, tick_t T, const Simulation& sim)
  {
    pa_[current_state_]->enter(this, idx, T, sim);
  }

  Prey::operator gl_agent_proxy() const noexcept
  {
    return gl_agent_proxy{
      glm::vec4(pos, 0.f, 0.f),
      glm::vec4(dir, 0.f, 0.f),
      glm::vec4(0)
    };
  }

  size_t Prey::update(size_t idx, tick_t T, const Simulation& sim)
  {
    old_dir = new_dir = dir;
    pa_[current_state_]->resume(this, idx, T, sim);
    last_update = T;
    return T + reaction_time;
  }

  void Prey::integrate(tick_t T, const Simulation& sim)
  {
    auto mix = math::smoothstep(float(T - last_update) / reaction_time, 0.f, 1.f);
    dir = math::slerp(old_dir, new_dir, mix);
    pos += sim.dt * speed_ * dir;
    pos = torus::wrap(WH, pos);
  }

  void Prey::on_state_exit(size_t idx, tick_t T, const Simulation& sim)
  {
    // select new state
    auto& dist = agent_discrete_dist;
    dist.mutate(T_[current_state_].cbegin(), T_[current_state_].cend());
    current_state_ = dist(reng);
    pa_[current_state_]->enter(this, idx, T, sim);
  }


  float Pred::WH;
  Pred::AP::transition_matrix Pred::T_;


  Pred::Pred(size_t idx, const json& J) :
    current_state_(0),
    speed_(J["speed"]),
    pos(0, 0),
    dir(1, 0),
    new_dir(1, 0),
    old_dir(1, 0)
  {
    if (idx == 0) {
      WH = J["WH"];
      T_ = J["T"];
    }
    pa_ = AP::create(idx, J["states"]);
  }

  void Pred::initialize(size_t idx, tick_t T, const Simulation& sim)
  {
    pa_[current_state_]->enter(this, idx, T, sim);
  }

  Pred::operator gl_agent_proxy() const noexcept
  {
    return gl_agent_proxy{
      glm::vec4(pos, 0.f, 0.f),
      glm::vec4(dir, 0.f, 0.f),
      glm::vec4(0)
    };
  }

  size_t Pred::update(size_t idx, tick_t T, const Simulation& sim)
  {
    old_dir = new_dir = dir;
    pa_[current_state_]->resume(this, idx, T, sim);
    last_update = T;
    return T + reaction_time;
  }

  void Pred::integrate(tick_t T, const Simulation& sim)
  {
    auto mix = math::smoothstep(float(T - last_update) / reaction_time, 0.f, 1.f);
    dir = math::slerp(old_dir, new_dir, mix);
    pos += sim.dt * speed_ * dir;
    pos = torus::wrap(WH, pos);
  }

  void Pred::on_state_exit(size_t idx, tick_t T, const Simulation& sim)
  {
    // select new state
    auto& dist = agent_discrete_dist;
    dist.mutate(T_[current_state_].cbegin(), T_[current_state_].cend());
    current_state_ = dist(reng);
    pa_[current_state_]->enter(this, idx, T, sim);
  }


}
