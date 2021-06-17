#ifndef INIT_CONDIT_HPP_INCLUDED
#define INIT_CONDIT_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <libs/torus.hpp>
#include <glmutils/random.hpp>
#include "model/model.hpp"
#include "model/simulation.hpp"
#include "model/json.hpp"


namespace initial_conditions {

  // config key: defined
  class defined_pos_dir
  { 
  public:
	defined_pos_dir(const json& J) :
    speed_(J["speed"]),
      pos0_(J["pos"][0], J["pos"][1]),
      dir0_(J["dir"][0], J["dir"][1]),
      radius_(J["radius"]),
      raddev_(glm::radians<float>(J["degdev"]))
    {}

    template <typename Entry>
    void operator()(Entry& entry)
    {
      auto uni = std::uniform_real_distribution<float>(0.f, 1.f);
      entry.pos = torus::wrap(model::Simulation::WH(), pos0_ + radius_ * model::vec_t(uni(model::reng), uni(model::reng)));
      const auto a = std::normal_distribution<float>(0, raddev_)(model::reng);
      const auto R = glm::mat2(std::cos(a), -std::sin(a), std::cos(a), std::sin(a));
	    entry.speed = speed_;
	    entry.dir = (R * dir0_);
    }

  private:
    model::vec_t pos0_;
    model::vec_t dir0_;
    float speed_;
    float radius_;
    float raddev_;
  };

  // config key: random
  class random_pos_dir
  {
  public:
	  random_pos_dir(const json& J)
	  {}

	  template <typename Entry>
	  void operator()(Entry& entry)
	  {
		  auto pdist = std::uniform_real_distribution<float>(0.f, model::Simulation::WH());
		  entry.pos = glm::vec2(pdist(model::reng), pdist(model::reng));
		  entry.dir = glmutils::unit_vec2(model::reng);
	  }
  };

  // config key: random_dead
  class random_dead
  {
  public:
    random_dead(const json& J)
    {}

    template <typename Entry>
    void operator()(Entry& entry)
    {
      auto pdist = std::uniform_real_distribution<float>(0.f, model::Simulation::WH());
      entry.pos = glm::vec2(pdist(model::reng), pdist(model::reng));
      entry.dir = glmutils::unit_vec2(model::reng);
      entry.alive = false;
    }
  };

  // config key: csv
  class from_csv
  { 
  public:
	 from_csv(const json& J) :
		csv_(std::filesystem::path(std::string(J["file"])))
		{
			csv_.ignore(2048, '\n');    // skip header
		}

    template <typename Entry>
    void operator()(Entry& entry)
    {
	  // function reads only one line
      Entry::stream_from_csv(csv_, entry);
	  // delete line that has been read already
	    csv_.ignore(2048, '\n'); 
    }

  private:
    std::ifstream csv_;
  };


  // config key: flock
  class in_flock
  {
  public:
	  in_flock(const json& J) :
      speed_(J["speed"]),
		  dir0_(J["dir"][0], J["dir"][1]),
		  radius_(J["radius"]),
		  raddev_(glm::radians<float>(J["degdev"]))
	  {}

	  template <typename Entry>
	  void operator()(Entry& entry)
	  {
		  auto uni = std::uniform_real_distribution<float>(0.f, 1.f);
		  entry.pos = torus::wrap(model::Simulation::WH(), radius_ * model::vec_t(uni(model::reng), uni(model::reng)));
		  const auto a = std::normal_distribution<float>(0, raddev_)(model::reng);
		  const auto R = glm::mat2(std::cos(a), -std::sin(a), std::cos(a), std::sin(a));
		  entry.dir = glm::normalize(R * dir0_);
		  entry.speed = speed_;
	  }

  private:
	  model::vec_t dir0_;
	  float speed_;
	  float radius_;
	  float raddev_;
  };

    //template <typename Agent>
    //void deviate_speed(Agent* self)
    //{  // individual variation in speeds
    //  auto spdist = std::normal_distribution<float>(0,4);
    //  self->cs_dev_ = spdist(model::reng); // cruise speed deviation
    //}
}
#endif
