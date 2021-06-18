#ifndef HUNT_ACTIONS_HPP_INCLUDED
#define HUNT_ACTIONS_HPP_INCLUDED

#include "model/action_base.hpp"
#include "agents/agents.hpp"
//#include "agents/agents.hpp"
//#include "agents/pigeon.hpp"

namespace model {
	namespace actions {

		template <typename Agent>
		class chase_closest_prey
		{ // go towards the closest individual from the flock // PIGEON SPECIFIC (pigeons tag in)

			make_action_from_this(chase_closest_prey);

		public:
			chase_closest_prey() {}
			chase_closest_prey(size_t, const json& J)
			{
				w_ = J["w"];                       // [1]
				prey_speed_scale_ = J["prey_speed_scale"];                       // [1]
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				const auto sv = sim.sorted_view<Tag, pigeon_tag>(idx);

				if (sv.size())
				{ 
					const auto& target = sim.pop<pigeon_tag>()[sv[0].idx]; // nearest prey
					auto ofss = torus::ofs(Simulation::WH(), self->pos, target.pos);;

					const auto Fdir = math::save_normalize(ofss, vec_t(0.f)) * w_;
					self->steering += Fdir;
					self->speed = prey_speed_scale_ * target.speed;
					self->target_i = static_cast<int>(sv[0].idx);
				}
			}

		private:
			float w_ = 0;           // [1]
			float prey_speed_scale_ = 0; // speed in relation to the preys speed [1]
		};


		template <typename Agent>
		class lock_on_closest_prey
		{ // lock on to the closest individual at entry and go towards it for the rest of the action // PIGEON SPECIFIC (pigeons tag in)

			make_action_from_this(lock_on_closest_prey);

		public:
			lock_on_closest_prey() {}
			lock_on_closest_prey(size_t, const json& J)
			{
				w_ = J["w"];                       // [1]
				prey_speed_scale_ = J["prey_speed_scale"];                       // [1]
				target_idx_ = -1;
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				const auto sv = sim.sorted_view<Tag, pigeon_tag>(idx);
				if (sv.size())
				{
					target_idx_ = sv[0].idx; // nearest prey
					self->target_i = static_cast<int>(target_idx_);
				}
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				if (target_idx_ != -1)
				{
					const auto& target = sim.pop<pigeon_tag>()[target_idx_]; // nearest prey
					auto ofss = torus::ofs(Simulation::WH(), self->pos, target.pos);;
					const auto Fdir = math::save_normalize(ofss, vec_t(0.f)) * w_;
					self->steering += Fdir;
					self->speed = prey_speed_scale_ * target.speed;
				}
			}

		private:
			float w_ = 0;           // [1]
			float prey_speed_scale_ = 0; // speed in relation to the preys speed [1]
			size_t target_idx_; //
		};


		template <typename Agent>
		class avoid_closest_prey
		{ // go towards the closest individual from the flock // PIGEON SPECIFIC (pigeons tag in)

			make_action_from_this(avoid_closest_prey);

		public:
			avoid_closest_prey() {}
			avoid_closest_prey(size_t, const json& J)
			{
				w_ = J["w"];                       // [1]
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				const auto sv = sim.sorted_view<Tag, pigeon_tag>(idx);

				if (sv.size())
				{
					const auto& flock_ind = sim.pop<pigeon_tag>()[sv[0].idx]; // nearest prey
					auto ofss = torus::ofs(Simulation::WH(), flock_ind.pos, self->pos);;

					const auto Fdir = math::save_normalize(ofss, vec_t(0.f)) * w_;
					self->steering += Fdir;
				}
			}

		public:
			float w_ = 0;           // [1]
		};

	}
}

#endif