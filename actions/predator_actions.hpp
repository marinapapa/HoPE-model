#ifndef PREDATOR_ACTIONS_HPP_INCLUDED
#define PREDATOR_ACTIONS_HPP_INCLUDED

#include "action_base.hpp"

namespace model {
  namespace actions {

		template <typename Agent>
		class set
		{
			make_action_from_this(set);

		public:
			set() {}
			set(size_t, const json& J)
			{
				pos_.x = J["pos"][0];
				pos_.y = J["pos"][1];
				dir_.x = J["dir"][0];
				dir_.y = J["dir"][1];
				speed_ = J["speed"];
				pos_ = torus::wrap(Simulation::WH(), pos_);
				dir_ = math::save_normalize(dir_, vec_t(1, 0));
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				self->pos = pos_;
				self->dir = dir_;
				self->speed = speed_;
			}

		private:
			pos_t pos_;
			vel_t dir_;
			float speed_;
		};


		template <typename Agent>
		class set_from_flock
		{
			make_action_from_this(set_from_flock);

		public:
			set_from_flock() {}
			set_from_flock(size_t, const json& J)
			{
				bearing_ = glm::radians(float(J["bearing"]));
				dist_ = J["distance"];
				prey_speed_scale_ = J["prey_speed_scale"];
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				const auto& target = sim.pop<pigeon_tag>()[self->target_f];
				self->pos = target.pos + dist_ * math::rotate(target.dir, bearing_);
				self->dir = target.dir;
				self->speed = prey_speed_scale_ * target.speed;
			}

		private:
			float bearing_;
			float dist_;
			float prey_speed_scale_;

		};


		template <typename Agent>
		class set_retreat
		{
			make_action_from_this(set_retreat);

		public:
			set_retreat() {}
			set_retreat(size_t, const json& J)
			{
				dist_away_ = J["distAway"];
				speed_ = J["speed"];
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{

			}
			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				self->pos = torus::wrap(Simulation::WH(), self->pos + dist_away_ * math::rotate(self->dir, math::pi<float>));
				self->dir = math::rotate(self->dir, math::pi<float>);
				self->speed = speed_;
			}

		private:
			float dist_away_;
			float speed_;
		};


		template <typename Agent>
		class hold
		{ // circle around position

			make_action_from_this(hold);

		public:
			hold() {}
			hold(size_t, const json& J)
			{
				pos_.x = J["pos"][0];
				pos_.y = J["pos"][1];
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
				auto ofs = torus::ofs(Simulation::WH(), self->pos, torus::wrap(Simulation::WH(), pos_));
				const auto Fdir = math::save_normalize(ofs, self->dir) * w_;
				self->steering += Fdir;
			}

		public:
			pos_t pos_;
			float w_;
		};

		template <typename Agent>
		class hold_current
		{ // circle around position

			make_action_from_this(hold_current);

		public:
			hold_current() {}
			hold_current(size_t, const json& J)
			{
				pos_ = glm::vec2(0.f, 0.f); // to be overwritten by current pos
				w_ = J["w"];                // [1]
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				pos_ = self->pos;
			}
			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				auto ofs = torus::ofs(Simulation::WH(), self->pos, torus::wrap(Simulation::WH(), pos_));
				const auto Fdir = math::save_normalize(ofs, self->dir) * w_;
				self->steering += Fdir;
			}

		public:
			pos_t pos_;
			float w_;
		};


		template <typename Agent>
		class waypoint
		{ // reach way-point

			make_action_from_this(waypoint);

		public:
			waypoint() {}
			waypoint(size_t, const json& J)
			{
				pos_.x = J["pos"][0];
				pos_.y = J["pos"][1];
				tolerance_ = J["tolerance"];
				tolerance_[0] = tolerance_[0] * tolerance_[0];
				tolerance_[1] = tolerance_[1] * tolerance_[1];
				tolerance_[2] = std::cos(glm::radians(tolerance_[2]));
				w_ = J["w"];
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				auto ofs = torus::ofs(Simulation::WH(), self->pos, torus::wrap(Simulation::WH(), pos_));
				const auto Fdir = math::save_normalize(ofs, self->dir);
				self->steering += w_ * Fdir;
				const auto dd = glm::length2(ofs);
				const auto b = std::abs(glm::dot(self->dir, Fdir));
				if ((dd < tolerance_[0]) && ((dd < tolerance_[1]) || (b < tolerance_[2]))) {
					self->on_state_exit(idx, T, sim);
				}
			}

		public:
			pos_t pos_;
			float w_;           // [1]
			std::array<float, 3> tolerance_;
		};


		template <typename Agent>
		class select_flock
		{
			make_action_from_this(select_flock);

			enum class Selection {
				Nearest,
				Biggest,
				Smallest,
				Random,
				MaxSelection
			};
		public:
			select_flock() {}
			select_flock(size_t, const json& J)
			{
				std::array<std::string, static_cast<size_t>(Selection::MaxSelection)> SelectionStr = {
					"nearest", "biggest", "smallest", "random"
				};
				const auto selstr = std::string(J["selection"]);
				auto it = std::find(SelectionStr.cbegin(), SelectionStr.cend(), selstr);
				if (it == SelectionStr.cend()) throw std::runtime_error("select_flock: unknown selection");
				selection_ = static_cast<Selection>(std::distance(SelectionStr.cbegin(), it));
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				select_target(self, sim);
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				select_target(self, sim);
			}

		private:
			void select_target(agent_type* self, const Simulation& sim)
			{
				const auto& flocks = sim.flocks<pigeon_tag>();
				auto it = flocks.cend();

				switch (selection_) {
				case Selection::Nearest:
					it = std::min_element(flocks.cbegin(), flocks.cend(), [pos = self->pos](const auto& a, const auto& b) {
						return torus::distance2(Simulation::WH(), a.gc(), pos) <
							torus::distance2(Simulation::WH(), b.gc(), pos);
					});
					break;
				case Selection::Biggest:
					it = std::max_element(flocks.cbegin(), flocks.cend(), [](const auto& a, const auto& b) {
						return a.size < b.size;
						});
					break;
				case Selection::Smallest:
					it = std::min_element(flocks.cbegin(), flocks.cend(), [](const auto& a, const auto& b) {
						return a.size < b.size;
						});
					break;
				case Selection::Random:
					if (!flocks.empty()) {
						auto dist = std::uniform_int_distribution<size_t>(0ull, flocks.size() - 1);
						it = flocks.cbegin() + dist(reng);
					}
					break;
				default:
					break;
				}
				self->target_f = -1;
				if (it != flocks.cend()) {
					const auto flock_id = static_cast<size_t>(std::distance(flocks.cbegin(), it));
					self->target_f = sim.flock_mates<pigeon_tag>(flock_id)[0];
				}
			}

			Selection selection_ = Selection::Nearest;
		};

		template <typename Agent>
		class shadowing
		{ // shadow closest prey

			make_action_from_this(shadowing);

		public:
			shadowing() {}
			shadowing(size_t, const json& J)
			{
				bearing_ = glm::radians(float(J["bearing"]));
				dist_ = J["distance"];
				placement_ = 0 != int(J["placement"]);
				w_ = J["w"];
				prey_speed_scale_ = J["prey_speed_scale"];            
			}

			void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				if (placement_ ) { // IF FIRST ACTION OF PREDATOR FAILS
					const auto& target = sim.pop<pigeon_tag>()[self->target_f];
					self->pos = target.pos + dist_ * math::rotate(target.dir, bearing_);
					self->dir = target.dir;
				}
			}

			void check_state_exit(const tick_t& state_dur, tick_t& state_exit_t)
			{
			}

			void operator()(agent_type* self, size_t idx, tick_t T, const Simulation& sim)
			{
				if (-1 != self->target_f) {
					const auto& target = sim.pop<pigeon_tag>()[self->target_f];
					const auto pos = target.pos + dist_ * math::rotate(target.dir, bearing_);
					const auto ofs = torus::ofs(Simulation::WH(), self->pos, torus::wrap(Simulation::WH(), pos));
					const auto Fdir = math::save_normalize(ofs, self->dir);
					self->steering += w_ * Fdir;
					self->speed = prey_speed_scale_ * target.speed;
				}
			}

		private:
			float bearing_;
			float dist_;
			float w_;           // [1]
			bool placement_;
			float prey_speed_scale_; // [1]
		};
	}
}


#endif
