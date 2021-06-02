#include <iostream>
#include "raubtier.hpp"


namespace model {

  namespace {
    thread_local rndutils::mutable_discrete_distribution<int, rndutils::all_zero_policy_uni> pred_discrete_dist;
  }

  // Raubtier

  decltype(Raubtier::transitions_) Raubtier::transitions_;
  flight::aero_info<float> Raubtier::ai_;
  const flight::aero_info<float>& Raubtier::ai = Raubtier::ai_;

  
  Raubtier::Raubtier(size_t idx, const json& J) :
    current_state_(0),
    rtau_(1.f),
    pos_(0, 0),
    dir_(1, 0)
  {
    if (idx == 0) {
      ai_ = flight::create_aero_info<float>(J["aero"]);
      transitions_ = decltype(transitions_)(J);
    }
    speed_ = cruiseSpeed_ = ai.cruiseSpeed;
    pa_ = AP::create(idx, J["states"]);
  }

  void Raubtier::initialize(size_t idx, const Simulation& sim)
  {
    pa_[current_state_]->enter(this, idx, 0, sim);
  }

  ::model::instance_proxy Raubtier::instance_proxy(long long color_map, size_t idx, const Simulation* sim) const noexcept
  {
    float tex = -1.f;
    switch (color_map) {
      case 1: tex = static_cast<float>(idx) / static_cast<float>(sim->pop<pred_tag>().size()); break;
      case 2: tex = glm::clamp(speed_ / Raubtier::ai.maxSpeed, 0.f, 1.f); break;
      case 3: {
        tex = 0.5f + glm::clamp(glm::degrees(bank()) / 90.f, -0.5f, 0.5f); break;
      }
      case 4: tex = float(current_state_) / AP::size; break;
    };
    return {
      glm::vec4(pos_, 0.f, tex),
      glm::vec4(dir_, 0.f, 1.f),
      glm::vec4(0)
    };
  }

  size_t Raubtier::update(size_t idx, tick_t T, const Simulation& sim)
  {
    //std::cout << glm::degrees(bank()) << ' ' << speed_ << '\n';
    steering_ = vec_t(0);
    pa_[current_state_]->resume(this, idx, T, sim);
    last_update = T;
    return T + reaction_time_;
  }

  void Raubtier::integrate(tick_t T, const Simulation& sim)
  {
    const float dt = Simulation::dt();
    const float hdt = 0.5f * dt;
    const float rBM = 1.0f / ai.bodyMass;
    
    // speed control
    const float sc = ai.bodyMass * rtau_ * (cruiseSpeed_ - speed_);
    steering_.x += sc;

    // modified Euler method (a.k.a. midpoint method)
    vec_t vel(speed_ * dir_);
    auto linForce = steering_.x * dir_;
    auto latForce = steering_.y * glmutils::perpDot(dir_);
    auto force = linForce + latForce;
    vel += accel_ * hdt;                       // v(t + dt/2) = v(t) + a(t) dt/2
    pos_ += vel * dt;                          // r(t + dt) = r(t) + v(t + dt/2)
    accel_ = (force_ + force) * rBM;           // a(t + dt) = F(t + dt)/m
    vel += accel_ * hdt;                       // v(t) = v(t + dt/2) + a(t + dt) dt/2

    // align direction with velocity
    speed_ = glm::length(vel);
    dir_ = vel / speed_;
    speed_ = glm::clamp(speed_, ai.minSpeed, ai.maxSpeed);
    pos_ = torus::wrap(Simulation::WH(), pos_);
  }

  void Raubtier::on_state_exit(size_t idx, tick_t T, const Simulation& sim)
  {
    // select new state
    auto& dist = pred_discrete_dist;
    const auto TM = transitions_(0.f);
    pred_discrete_dist.mutate(TM[current_state_].cbegin(), TM[current_state_].cend());
    current_state_ = pred_discrete_dist(reng);
    pa_[current_state_]->enter(this, idx, T, sim);
  }

  float Raubtier::bank() const
  {
    const float bodyWeight = 9.81f * ai.bodyMass;
    const float L = bodyWeight * (speed_ * speed_) / (ai.cruiseSpeed * ai.cruiseSpeed);  // Lift
    const auto latForce = steering_.y;
    const auto alpha = std::asin(latForce / L);
    return alpha;
  }
}
