#ifndef MODEL_SIMULATION_HPP_INCLUDED
#define MODEL_SIMULATION_HPP_INCLUDED

#include <mutex>
#include <atomic>
#include <json/json.hpp>
#include "flock.hpp"


namespace model {


  class Simulation
  {
  private:
    static float WH_;
    static float dt_;

  public:
    enum Msg {
      Tick = 0,
      Initialized,
      Finished,
      MaxMsg
    };

  public:
    explicit Simulation(const json& J);
    ~Simulation();
    void initialize(class Observer* observer, const species_snapshots& ss);
    void set_snapshots(const species_snapshots& ss);
    species_snapshots get_snapshots() const;

    void update(class Observer* observer);
    
    static float WH() noexcept { return WH_; }
    static float dt() noexcept { return dt_; }      // [s]

    tick_t tick() const noexcept { return tick_; }  // [1]
    tick_t time2tick(double time) const noexcept { return static_cast<tick_t>(time / dt_); }  // [1]
    double time() const noexcept { return static_cast<double>(dt_) * tick_; }                 // [s]
    double tick2time(tick_t tick) const noexcept { return static_cast<double>(dt_) * tick_; } // [s]

    // returns const reference to population vector that
    // contains both alive and non-alive individuals.
    template <typename Tag>
    const auto& pop() const noexcept 
    {
      return std::get<Tag::value>(species_);
    }

    // returns exclusive (alive) neighborhood sorted by distance
    template <typename Tag, typename OtherTag = Tag>
    neighbor_info_view sorted_view(size_t idx) const noexcept
    {
      return sorted_view_impl(idx, Tag::value, OtherTag::value);
    }

    template <typename Tag>
    const size_t& are_alive() const noexcept
    {
      return state_[Tag::value].alive;
    }

    template <typename Tag>
    const std::vector<flock_descr>& flocks() const noexcept
    {
      return state_[Tag::value].flock_tracker.flocks();
    }

    template <typename Tag>
    flock_descr flock_info(size_t flock_id) const
    {
      return state_[Tag::value].flock_tracker.descr(static_cast<int>(flock_id));
    }

    template <typename Tag>
    int flock_of(size_t idx) const
    {
      return state_[Tag::value].flock_tracker.id_of(idx);
    }

    template <typename Tag>
    std::vector<int> flock_mates(size_t flock_id) const
    {
      auto fm = std::vector<int>{};
      for (size_t i = 0; i < state_[Tag::value].flock_tracker.pop_size(); ++i) {
        if (flock_of<Tag>(i) == flock_id) {
          fm.push_back(static_cast<int>(i));
        }
      }
      return fm;
    }

    // Access from foreign threads
    
    void terminate() const noexcept { terminate_.store(true, std::memory_order_release); }
    bool terminated() const noexcept { return terminate_.load(std::memory_order_acquire); }

    // calls fun for all individuals, internally synchronized
    template <typename Tag, typename Fun>
    size_t visit_all(Fun&& fun) const
    {
      std::lock_guard<std::recursive_mutex> _(mutex_);
      auto& pop = std::get<Tag::value>(species_);
      size_t n = 0;
      for (size_t i = 0; i < pop.size(); ++i) {
        bool alive = state_[Tag::value].update_times[i] != static_cast<tick_t>(-1);
        fun(pop[i], i, alive); ++n;
      }
      return n;
    }

    // calls fun for all alive individuals, internally synchronized
    template <typename Tag, typename Fun>
    size_t visit(Fun&& fun) const
    {
      std::lock_guard<std::recursive_mutex> _(mutex_);
      auto& pop = std::get<Tag::value>(species_);
      size_t n = 0;
      for (size_t i = 0; i < pop.size(); ++i) {
        if (state_[Tag::value].update_times[i] != static_cast<tick_t>(-1)) {
          fun(pop[i]); ++n;
        }
      }
      return n;
    }

    // calls fun for individual idx if alive, internally synchronized
    template <typename Tag, typename Fun>
    size_t visit(size_t idx, Fun&& fun) const
    {
      std::lock_guard<std::recursive_mutex> _(mutex_);
      auto& pop = std::get<Tag::value>(species_);
      assert(idx < pop.size());
      size_t n = 0;
      if (state_[Tag::value].update_times[idx] != static_cast<tick_t>(-1)) {
        fun(pop[idx]); ++n;
      }
      return n;
    }

    template <typename Tag>
    bool is_alive(size_t idx) const noexcept
    {
      std::lock_guard<std::recursive_mutex> _(mutex_);
      return (state_[Tag::value].update_times[idx] != static_cast<tick_t>(-1));
    }

    // sets alive flag for all individuals, internally synchronized
    template <typename Tag>
    void set_alive(bool alive) const
    {
      std::lock_guard<std::recursive_mutex> _(mutex_);
      if (alive) {
        auto reng = rndutils::make_random_engine_low_entropy<>();
        auto udist = std::uniform_real_distribution<>(0.0, 1.0 / double(dt_));
        for (auto& ut : state_[Tag::value].update_times) {
          ut = tick_ + static_cast<tick_t>(udist(reng));
        }
        return;
      }
      for (auto& ut : state_[Tag::value].update_times) {
        ut = static_cast<tick_t>(-1);
      }
    }

    // sets alive flag for individual idx, internally synchronized
    template <typename Tag>
    void set_alive(size_t idx, bool alive) const
    {
      std::lock_guard<std::recursive_mutex> _(mutex_);
      assert(idx < state_[Tag::value].update_times.size());
      if (alive) {
        auto reng = rndutils::make_random_engine_low_entropy<>();
        state_[Tag::value].update_times[idx] = tick_ + static_cast<tick_t>(std::uniform_real_distribution<>(0.0, 1.0 / double(dt_))(reng));
        return;
      }
      state_[Tag::value].update_times[idx] = static_cast<tick_t>(-1);
    }

    // increases stress for individual idx, internally synchronized
    template <typename Tag>
    void petrubate_alive() const
    {
    }

  private:
    // returns exclusive (alive) neighborhood sorted by distance
    neighbor_info_view sorted_view_impl(size_t idx, size_t S1, size_t S2) const noexcept
    {
      const auto alive = state_[S2].alive;
      const auto n = state_[S2].update_times.size();
      if (alive) {
        const auto first = &state_[S1].NI[S2][idx * n + (n - alive)];
        if (S1 == S2) {
          return neighbor_info_view{ first + 1, alive - 1 };
        }
        return { first, alive };
      }
      return { nullptr, 0 };
    }


  private:
    tick_t tick_ = 0;
    tick_t flock_update_ = 0;
    tick_t flock_interval_ = 0;
    float flock_dd_ = 0.f;
    mutable std::recursive_mutex mutex_;      // simulation lock
    mutable species_pop species_;
    mutable std::atomic<bool> terminate_ = false;

    struct state_t
    {
      size_t alive;   // number of alive ind
      std::vector<tick_t> update_times;
      std::array<std::vector<neighbor_info>, n_species> NI;   // neighbor info matrices
      flock_tracker flock_tracker;
    };
    mutable std::array<state_t, n_species> state_;
    friend class flock_tracker;

   public:
     using state_array = decltype(state_);
   };

}

#endif
