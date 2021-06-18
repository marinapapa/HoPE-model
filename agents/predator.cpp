#include "predator.hpp"
#include "pigeon.hpp"
#include "model/init_cond.hpp"


namespace model {

  namespace {
    thread_local rndutils::mutable_discrete_distribution<int, rndutils::all_zero_policy_uni> pred_discrete_dist;
  }

  // Predator

  decltype(Pred::transitions_) Pred::transitions_;
  // flight::aero_info<float> Pred::ai;
  //const flight::aero_info<float>& Pred::ai = Pred::ai;


  template <typename Init>
  void do_init_pop(std::vector<snapshot_entry<pred_tag>>& vse, Init&& init)
  {
    for (auto& e : vse) init(e);
  }  


  std::vector<snapshot_entry<pred_tag>> Pred::init_pop(const Simulation& sim, const json& J)
  {
    const size_t N = J["N"];
    auto jic = J["InitCondit"];
    std::string type = jic["type"];
    if (type == "none") return {};
    std::vector<snapshot_entry<pred_tag>> vse(N);
    if (type == "random") do_init_pop(vse, initial_conditions::random_pos_dir(jic));
    else throw std::runtime_error("unknown initializer");
    return vse;
  }


  Pred::Pred(size_t idx, const json& J) :
    current_state_(0),
    target_i(-1),
    target_f(-1),
    pos(0, 0),
    dir(1, 0),
    accel(0) // [m / s^2]
  {
    if (idx == 0) {
      transitions_ = decltype(transitions_)(J);
    }
    ai = flight::create_aero_info<float>(J["aero"]);
    speed = sa.cruiseSpeed = ai.cruiseSpeed;
    sa.w = 0.f; // until they get value from state? (first integrates before update)
    pa_ = AP::create(idx, J["states"]);
  }

  void Pred::initialize(size_t idx, const Simulation& sim, const json& J)
  {
    pa_[current_state_]->enter(this, idx, 0, sim);
  }

  ::model::instance_proxy Pred::instance_proxy(long long color_map, size_t idx, const Simulation* sim) const noexcept
  {
    float tex = -1.f;
    switch (color_map) {
      case 1: tex = speed / 30.f; break;
      case 2: tex = float(current_state_) / AP::size; break;
    };
    tex = std::clamp(tex, -1.f, 1.f);  // yes -1,+1, need '-1' in shader
    return { pos, speed * dir, glmutils::perpDot(dir), tex };
  }

  ::model::snapshot_entry<pred_tag> Pred::snapshot(const Simulation* sim, size_t idx) const noexcept
  {
    return { pos,  dir, speed , accel, sim->is_alive<Tag>(idx) };
  }

  void Pred::snapshot(Simulation* sim, size_t idx, const snapshot_entry<pred_tag>& se) noexcept
  {
    pos = se.pos;
    speed = se.speed;
    dir = se.dir;
    accel = se.accel;
    sim->set_alive<Tag>(idx, se.alive);
  }

  size_t Pred::update(size_t idx, tick_t T, const Simulation& sim)
  {
    steering = vec_t(0);
    pa_[current_state_]->resume(this, idx, T, sim);
    last_update = T;
    return T + reaction_time;
  }

  void Pred::integrate(tick_t T, const Simulation& sim)
  {
    flight_control::integrate_motion(this);
  }

  void Pred::on_state_exit(size_t idx, tick_t T, const Simulation& sim)
  {
    target_i = -1;
    // select new state
    auto& dist = pred_discrete_dist;
    const auto TM = transitions_(0.f);
    pred_discrete_dist.mutate(TM[current_state_].cbegin(), TM[current_state_].cend());
    current_state_ = pred_discrete_dist(reng);
    pa_[current_state_]->enter(this, idx, T, sim);
  }
}
