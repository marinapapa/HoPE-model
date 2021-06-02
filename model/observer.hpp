#ifndef MODEL_OBSERVER_HPP_INCLUDED
#define MODEL_OBSERVER_HPP_INCLUDED

#include <deque>

namespace model {

  class Observer
  {
  public:
    virtual ~Observer() {};
    virtual void notify(long long msg, const class Simulation& sim)
    {
      notify_next(msg, sim);
    }

    void notify_next(long long msg, const class Simulation& sim)
    {
      if (next_) next_->notify(msg, sim);
    }

    void append_observer(Observer* observer)
    {
      if (next_) next_->append_observer(observer);
      else next_ = observer;
    }

    virtual void notify_once(const class Simulation& sim)
    {
      if (next_) next_->notify_once(sim);
    }

  private:
    Observer* next_ = nullptr;
    Observer* parent_ = nullptr;
  };


  class AnalysisObserver : public model::Observer
  {
  public:

      AnalysisObserver(const std::filesystem::path& out_path, const json& J)
      {
          const std::string out_name = J["output_name"];
          full_out_path_ = (out_path / (out_name + ".csv")).string();
          const float freq_sec = J["sample_freq"];
          oi_.sample_tick = oi_.sample_freq = static_cast<tick_t>(freq_sec / model::Simulation::dt());
      }
      virtual ~AnalysisObserver() {};

      struct obs_info
      {
          tick_t sample_freq;
          tick_t sample_tick;
      };

	  void notify(long long lmsg, const model::Simulation& sim)
	  {
		  using Msg = model::Simulation::Msg;
		  auto msg = Msg(lmsg);

		  switch (msg) {
		  case Msg::Tick: {
			  if (sim.tick() >= oi_.sample_tick)
			  {
				  notify_collect(sim);
				  oi_.sample_tick = sim.tick() + oi_.sample_freq;
			  }
			  if (data_out_.size() > 10000) // avoid overflow 
			  {
				  notify_save(sim);
				  data_out_.clear();
			  }
			  break;
		  }
		  case Msg::Finished:
			  notify_save(sim);
			  break;
		  }
		  notify_next(lmsg, sim);
	  }

	  virtual void notify_collect(const model::Simulation& sim) {};
	  virtual void notify_save(const model::Simulation& sim) {};
	   
  protected:
	   obs_info oi_;
	   std::deque<std::vector<float>> data_out_;
       std::ofstream outfile_stream_;
       std::string full_out_path_;
  };

}

#endif
