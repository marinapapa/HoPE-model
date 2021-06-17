#ifndef AVOID_PRED_ACTIONS_HPP_INCLUDED
#define AVOID_PRED_ACTIONS_HPP_INCLUDED

#include "model/action_base.hpp"
#include <glmutils/ray.hpp>
#include <glmutils/random.hpp>
#include "model/while_topo.hpp"
#include "agents/agents.hpp"

namespace model {
  namespace actions {

    // PREDATOR - PREY INTERACTIONS 
    template <typename Agent>
    class avoid_p_position
    { // avoid predators position

      make_action_from_this(avoid_p_position);

    public:
      avoid_p_position() {}
      avoid_p_position(size_t, const json& J)
      {
        float minsep = J["minsep"];       // [m]
        minsep2 = minsep * minsep;        // [m^2]

        w_ = J["w"];                       // [1]
      }

      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
      }

      void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
      {
        const auto nv = sim.sorted_view<Tag, pred_tag>(idx);

        // WITHOUT FOV APPLIED - ?
        if (nv.size() && (nv[0].dist2 < minsep2))
        {
          const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
		  if (predator.target_i >= 0 && static_cast<size_t>(predator.target_i) == idx) { self->am_target = true; } else { self->am_target = false; }
          const auto ofss = torus::ofs(Simulation::WH(), predator.pos, self->pos);
		  const auto Fdir = math::save_normalize(ofss, vec_t(0.f)) * w_;
		  self->steering += Fdir;
        }
      }

		public:
      float minsep2 = 0;  // [m^2]

		private:
      float w_ = 0;       // [1]
    };

	// PREDATOR - PREY INTERACTIONS 
	template <typename Agent>
	class avoid_p_direction
	{ // avoid predators position

		make_action_from_this(avoid_p_direction);

	public:
		avoid_p_direction() {}
		avoid_p_direction(size_t, const json& J)
		{
			float minsep = J["minsep"];       // [m]
			minsep2 = minsep * minsep;        // [m^2]

			w_ = J["w"];                       // [1]
		}

		void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
		}

		void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
			const auto nv = sim.sorted_view<Tag, pred_tag>(idx);

			// WITHOUT FOV APPLIED - ?
			if (nv.size() && (nv[0].dist2 < minsep2))
			{
				const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
				const float rad_away_pred = math::rad_between(predator.dir, self->dir);
				if (predator.target_i >= 0 && static_cast<size_t>(predator.target_i) == idx) { self->am_target = true; }
				else { self->am_target = false; }
				auto w = std::copysignf(w_, rad_away_pred);
				self->steering += glmutils::perpDot(self->dir) * w;
			}
		}

	public:
		float minsep2 = 0;  // [m^2]

	private:
		float w_ = 0;       // [1]
	};

	//turn in time as reaction to a predator
	template <typename Agent>
	class t_turn_pred
	{
		make_action_from_this(t_turn_pred);

	public:
		t_turn_pred() {}
		t_turn_pred(size_t, const json& J)
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

			// find direction away from predator
			const auto nv = sim.sorted_view<Tag, pred_tag>(idx); 

			if (nv.size())
			{
				const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
				if (predator.target_i >= 0 && static_cast<size_t>(predator.target_i) == idx) { self->am_target = true; }
				else { self->am_target = false; }
				auto dir_away = glm::normalize(torus::ofs(Simulation::WH(), predator.pos, self->pos));
				w_ = (glmutils::perpDot(self->dir, dir_away) > 0) ? 1.f : -1.f; // perp dot positive, b on right of a (for perpdot(a,b))
			}
			else
			{
				w_ = 0.f;
			}

		}

		void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
			// Fz = m * v*v/r 
			turn_dir_ = w_ * glmutils::perpDot(self->dir);
			auto Fz = self->ai.bodyMass * self->speed * self->speed / r_;
			self->steering += Fz * turn_dir_;
		}

	private:
		float r_ = 0;
		vec_t turn_dir_;
		float turn_ = 0;   // [rad]
		float time_ = 0;
		float w_ = 0;      // [1] 
	};


	//turn random degrees within a window in given time as reaction to a predator
	template <typename Agent>
	class random_t_turn_pred
	{
		make_action_from_this(random_t_turn_pred);

	public:
		random_t_turn_pred() {}
		random_t_turn_pred(size_t, const json& J)
		{
			const float turn_min = glm::radians(float(J["turn_min"]));
			const float turn_max = glm::radians(float(J["turn_max"]));
			const float time_min = J["time_min"];
			const float time_max = J["time_max"];

			if ( time_min <= 0.f ||  time_max <= 0.f || turn_max == 0.f) throw std::runtime_error("wrong parameters in random_t_turn");

			turn_distr_ = std::uniform_real_distribution<float>(turn_min, turn_max);
			time_distr_ = std::uniform_real_distribution<float>(time_min, time_max);
			turn_dur_ = static_cast<tick_t>(time_min / Simulation::dt());
		}
		void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
		{
			if (state_dur > turn_dur_) { state_exit_t -= (state_dur - turn_dur_); }
		}

		void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
			// we want to turn turn_ radians in time_ seconds.
			const auto thisturn = turn_distr_(model::reng);
			auto loc_time = time_distr_(model::reng);
			turn_dur_ = static_cast<tick_t>(static_cast<double>(loc_time) / Simulation::dt());

			auto w = thisturn / loc_time;       // required angular velocity
			r_ = self->speed / w;       // radius

			// find direction away from predator
			const auto nv = sim.sorted_view<Tag, pred_tag>(idx);

			if (nv.size())
			{
				const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
				//auto dir_away = glm::normalize(torus::ofs(Simulation::WH(), predator.pos, self->pos));
				//w_ = (glmutils::perpDot(self->dir, dir_away) > 0) ? 1.f : -1.f; // perp dot positive, b on right of a (for perpdot(a,b))
				if (predator.target_i >= 0 && static_cast<size_t>(predator.target_i) == idx) { self->am_target = true; }
				else { self->am_target = false; }
				const float rad_away_pred = math::rad_between(predator.dir, self->dir);
				w_ = std::copysignf(1.f, rad_away_pred);
			}
			else
			{
				w_ = 0.f;
			}
		}

		void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
			// Fz = m * v*v/r 
			turn_dir_ = w_ * glmutils::perpDot(self->dir);
			auto Fz = self->ai.bodyMass * self->speed * self->speed / r_;
			self->steering += Fz * turn_dir_;

		}

	private:
		float r_ = 0;
		vec_t turn_dir_;
		tick_t turn_dur_;
		std::uniform_real_distribution<float> turn_distr_;
		std::uniform_real_distribution<float> time_distr_;
		float w_ = 0;      // [1] 
	};

	//turn random degrees within a window in given time as reaction to a predator
	template <typename Agent>
	class random_t_turn_gamma_pred
	{
		make_action_from_this(random_t_turn_gamma_pred);

	public:
		random_t_turn_gamma_pred() {}
		random_t_turn_gamma_pred(size_t, const json& J)
		{
			const float turn_mean = glm::radians(float(J["turn_mean"]));
			const float turn_sd = glm::radians(float(J["turn_sd"]));
			const float time_mean = J["time_mean"];
			const float time_sd = J["time_sd"];

			if (turn_mean <= 0.f || turn_sd <= 0.f || time_mean <= 0.f || turn_mean == 0.f) throw std::runtime_error("wrong parameters in random_t_turn");

			const float turn_alpha = (turn_mean / turn_sd) * (turn_mean / turn_sd);
			const float turn_beta = (turn_sd * turn_sd) / turn_mean ;

			const float time_alpha = (time_mean / time_sd) * (time_mean / time_sd);
			const float time_beta = (time_sd * time_sd) / time_mean;

			turn_distr_ = std::gamma_distribution<float>(turn_alpha, turn_beta);
			time_distr_ = std::gamma_distribution<float>(time_alpha, time_beta);

			turn_dur_ = static_cast<tick_t>(turn_mean / Simulation::dt());
		}
		void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
		{
			if (state_dur > turn_dur_) { state_exit_t -= (state_dur - turn_dur_); }
		}

		void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
			// we want to turn turn_ radians in time_ seconds.
			auto loc_time = 0.f; // random to initialize
			auto thisturn = 0.f; // random to initialize
			do {
				loc_time = time_distr_(model::reng);
				thisturn = turn_distr_(model::reng);
			} while ( loc_time * thisturn <= 0.f ); // both not 0

			turn_dur_ = static_cast<tick_t>(static_cast<double>(loc_time) / Simulation::dt());

			auto w = thisturn / loc_time;       // required angular velocity
			r_ = self->speed / w;       // radius

			// find direction away from predator
			const auto nv = sim.sorted_view<Tag, pred_tag>(idx);

			if (nv.size())
			{
				const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
				const float rad_away_pred = math::rad_between(predator.dir, self->dir);
				w_ = std::copysignf(1.f, rad_away_pred);
			}
			else
			{
				w_ = 0.f;
			}
		}

		void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
		{
			// Fz = m * v*v/r 
			turn_dir_ = w_ * glmutils::perpDot(self->dir);
			auto Fz = self->ai.bodyMass * self->speed * self->speed / r_;
			self->steering += Fz * turn_dir_;

		}

	private:
		float r_ = 0;
		vec_t turn_dir_;
		tick_t turn_dur_;
		std::gamma_distribution<float> turn_distr_;
		std::gamma_distribution<float> time_distr_;
		float w_ = 0;      // [1] 
	};

	//turn in time as reaction to a predator
	template <typename Agent>
	class zig_zag
	{
		make_action_from_this(zig_zag);

	public:
		zig_zag() {}
		zig_zag(size_t, const json& J)
		{
			turn_ = glm::radians(float(J["turn"]));
			time_ = J["time"];

			if (time_ == 0.f || turn_ == 0.f) throw std::runtime_error("wrong parameters in zig zag");

			zig_timer_ = static_cast<tick_t>(time_ / (Simulation::dt() * 2.f)); // [ticks for each subturn]
			w_ = 1.f; // control change in direction
		}

		void on_entry(agent_type * self, size_t idx, tick_t T, const Simulation & sim)
		{
			// to signal exit of first zig
			entry_tick_ = T;

			// we want to turn turn_ radians in time_ seconds.
			auto w = 2.f * turn_ / time_;       // required angular velocity
			r_ = self->speed / w;       // radius
		
			//find direction away from predator
			const auto nv = sim.sorted_view<Tag, pred_tag>(idx); // add through runtime error if no predator? SHOULD CHECK IF PREDATOR? BUT SHOULD ONLY BE WHEN PREDATOR AROUND ANYWAYS
			
			// cause predator might be dead during a turn
			if (nv.size())
			{
				const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
				if (predator.target_i >= 0 && static_cast<size_t>(predator.target_i) == idx) { self->am_target = true; }
				else { self->am_target = false; }
				auto dir_away = glm::normalize(torus::ofs(Simulation::WH(), predator.pos, self->pos));
				w_ = (glmutils::perpDot(self->dir, dir_away) > 0) ? 1.f : -1.f; // dot positive, b on right of a (for dot(a,b))
			}
			else
			{
				w_ = 0.f; // no predator in the simulation
			}
		
		}

		void operator()(agent_type * self, size_t idx, tick_t T, const Simulation & sim)
		{
			// Fz = m * v*v/r
			turn_dir_ = w_ * glmutils::perpDot(self->dir);

			auto Fz = self->ai.bodyMass * self->speed * self->speed / r_;
			self->steering += Fz * turn_dir_;

			if ((T - entry_tick_) > zig_timer_) 
			{
				w_ = -w_;
				entry_tick_ = T;		// restart count for next zig
			}
		}

	private:
		float r_ = 0;
		vec_t turn_dir_;
		tick_t zig_timer_ = 0; // count when it finished the first turn to switch turn_dir [ticks]
		tick_t entry_tick_ = 0; 
		float turn_ = 0;   // [rad]
		float time_ = 0;
		float w_ = 0;      // [1]
	};

  }
}


#endif