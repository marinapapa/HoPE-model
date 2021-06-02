#include <filesystem>
#include <iostream>
#include <future>
#include <thread>
#include <tbb/tbb.h>
#include <json/json.hpp>
#include "model/model.hpp"
#include "simgl/AppWin.h"
#include "analysis/analysis_obs.hpp"
#include <libs/cmd_line.h>


// model thread function
void run_simulation(model::Simulation* sim, 
                    const species_snapshots& ss,
                    model::Observer* observer, 
					          //analysis::DataExporter* dataexp,
                    const json& J)
{
  try {
    int numThreads = J["Simulation"]["numThreads"];
    if (numThreads == -1) numThreads = tbb::task_scheduler_init::default_num_threads();
    numThreads = std::clamp(numThreads, 1, tbb::task_scheduler_init::default_num_threads());
    tbb::task_scheduler_init tbb_init(numThreads);
    auto Tmax = sim->time2tick(double(J["Simulation"]["Tmax"]));
    sim->initialize(observer, ss);
    while (!sim->terminated()) {
      sim->update(observer);
      if (sim->tick() == Tmax) {
        break;
      }
    }
    observer->notify(model::Simulation::Finished, *sim);
  }
  catch (std::exception& err) {
    observer->notify(model::Simulation::Finished, *sim);
    std::cerr << err.what() << '\n';
  }
}


// should come from file or something...
const static species_snapshots initial_snapshot = {
  {},                                  // pigeons
  {}                                   // predator(s)
};


void run(json& J, bool headless)
{
  model::species_snapshots ss = initial_snapshot;
  for (;;) {
    auto sim = std::make_unique<model::Simulation>(J);
    auto observers = analysis::CreateObserverChain<model::pigeon_tag>(J);
    if (headless) {
      auto observer  = std::make_unique<Observer>();
      std::for_each(observers.begin(), observers.end(), [&](const std::unique_ptr<Observer>& obs) {
        observer->append_observer(obs.get());
      });
      run_simulation(sim.get(), ss, observer.get(), J);
      break;
    }
    else {
      auto appWin = std::make_unique<AppWin>();
      std::for_each(observers.begin(), observers.end(), [&appWin](const std::unique_ptr<Observer>& obs) {
        appWin->append_observer(obs.get());
      });
      auto future = std::async(std::launch::async, [&]() { run_simulation(sim.get(), ss, appWin.get(), J); });
      int retval = appWin->AppLoop(sim.get(), J);
      if (future.valid()) {
        future.get();
      }
      if (retval == 0) break;
      else if (retval == 1) ss = sim->get_snapshots();    // restart with current snapshot
      else if (retval == 2) ss = initial_snapshot;        // restart from scratch
    }
  }
}


int main(int argc, const char* argv[])
{
  try {
    auto clp = cmd::cmd_line_parser(argc, argv);
    std::vector<std::filesystem::path> configs;
    std::string config_name;
	if (std::filesystem::path config = ""; clp.optional("config", config)) {
	  configs.assign(1, config);
      config_name = (config).string();
	  }
    else {
      //configs = { std::filesystem::path("config_test_1.json") } ;
        configs = {
          std::filesystem::path("config.json") ,
          std::filesystem::path("species/pigeon.json"),
          std::filesystem::path("species/predator.json")
        };
      config_name = "composed_config.json";
    }

    auto J = compose_json(configs);
    J["Simulation"]["Analysis"]["Externals"]["configName"] = config_name;
    
    std::string exp_files;
    clp.optional("exp_files", exp_files);
    if (exp_files == "true") {
        save_json(J, "composed_config.json");
    }
    
    run(J, clp.flag("--headless"));
    return 0;
  }
  catch (const std::exception& err) {
    std::cerr << err.what();
  }
  return -1;
}
