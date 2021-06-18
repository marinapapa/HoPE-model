#ifndef ANALYSIS_HPP_INCLUDED
#define ANALYSIS_HPP_INCLUDED

#include <iostream>
#ifdef WIN32
#include "simgl/AppWin.h"
#endif
#include <filesystem>
#include <deque>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include "model/flock.hpp"
#include "model/simulation.hpp"
#include "model/observer.hpp"
#include "math.hpp"
#include "torus.hpp"
using namespace model;

namespace analysis
{
	using timeseries_t = std::deque<std::vector<float>>;
	using path_t = std::filesystem::path;

	template <typename Agent>
  inline float head_dif(const Agent& p, const model::flock_descr& f)
  {
		return math::rad_between(p.dir, f.vel);
  }

	template <typename PreyAgent, typename PredAgent>
	inline float in_conflict_pos(const PreyAgent& prey, const PredAgent& pred, const model::flock_descr& f)
	{
		auto dir_towards_fl = glm::normalize(torus::ofs(Simulation::WH(), prey.pos, f.gc()));
		auto dir_away_pred = glm::normalize(torus::ofs(Simulation::WH(), pred.pos, prey.pos));

		auto dot_p = (glmutils::perpDot(prey.dir, dir_away_pred) > 0) ? 0 : 1;
		auto dot_f = (glmutils::perpDot(prey.dir, dir_towards_fl) > 0) ? 0 : 1;
		return (dot_p != dot_f) ? 1.f : 0.f ;
	}

	// conflict based on direction towards coherence (center of flock)
	template <typename PreyAgent, typename PredAgent>
	inline float in_conflict_dir_coh(const PreyAgent& prey, const PredAgent& pred, const model::flock_descr& f)
	{
		auto dir_towards_fl = glm::normalize(torus::ofs(Simulation::WH(), prey.pos, f.gc()));
		auto rad_away_pred = math::rad_between(pred.dir, prey.dir);
		auto rad_to_fl = math::rad_between(prey.dir, dir_towards_fl);

		return ((rad_away_pred * rad_to_fl) < 0) ? 1.f : 0.f;
	}

	// conflict based on direction towards alignment (average direction of flock)
	template <typename PreyAgent, typename PredAgent>
	inline float in_conflict_dir_ali(const PreyAgent& prey, const PredAgent& pred, const model::flock_descr& f)
	{
		auto rad_away_pred = math::rad_between(pred.dir, prey.dir);
		auto rad_to_fl = math::rad_between(prey.dir, f.vel);

		return ((rad_away_pred * rad_to_fl) < 0) ? 1.f : 0.f;
	}

	template <typename Agent>
	inline float centrality(const Agent& pf, const size_t& idxf, const model::Simulation& sim)
	{
		//const auto sv = sim.sorted_view<pigeon_tag>(idxf);
		//const auto& flock = sim.pop<pigeon_tag>();
		//vec_t adir(0.f);
		//auto n = 0.f; // number of neighbors
		//for (auto it = sv.cbegin(); it != sv.cend(); ++it) {
		//	if (sim.flock_of<pigeon_tag>(idxf) == sim.flock_of<pigeon_tag>(it->idx))
		//	{
		//		adir += torus::ofs(Simulation::WH(), pf.pos, flock[it->idx].pos);
		//		++n;
		//	}
		//}

		vec_t adir(0.f);
		auto n = 0.f; // number of neighbors
		sim.visit_all<pigeon_tag>([&](auto& p, size_t idx, bool alive) {
			if (alive && idx != idxf)
			{
				if (sim.flock_of<pigeon_tag>(idxf) == sim.flock_of<pigeon_tag>(idx))
				{
					adir += torus::ofs(Simulation::WH(), pf.pos, p.pos);
					++n;
				}	
			}
		});

		if (n)
		{
			return glm::length(adir / n);
		}
		return 0.f;
	}

	inline path_t output_path(const json& J)
	{
		auto exePath = std::filesystem::current_path();
		path_t top_folder = exePath;

		std::string main_dat_folder = "simulated_data";
		std::string outf = J["data_folder"];


		path_t filefolder = (top_folder / main_dat_folder).string();
		if (!(std::filesystem::exists(filefolder)))
		{
			std::filesystem::create_directory(filefolder.string());
		}
		filefolder = (filefolder / outf).string();
		if (!(std::filesystem::exists(filefolder)))
		{
			std::filesystem::create_directory(filefolder.string());
		}
		return filefolder;
	}

	inline path_t unique_output_folder(const json& J)
	{
		auto distr = std::uniform_int_distribution<int>(0, 1000);
		const auto random_id = std::to_string(distr(model::reng));

		struct std::tm local_time;
	    const std::time_t now = time(0);
#ifdef WIN32
		localtime_s(&local_time, &now);
#else
		localtime_r(&now, &local_time);
#endif
		const time_t ttime = time(0);

		const std::string thyear = std::to_string(1900 + local_time.tm_year);
		const std::string thmonth = std::to_string(1 + local_time.tm_mon) + std::to_string(local_time.tm_mday);
		const std::string thtime = std::to_string(local_time.tm_hour) + std::to_string(local_time.tm_min) + std::to_string(local_time.tm_sec);
		const std::string full_name = thyear + thmonth + thtime + std::to_string(ttime) + random_id;

		const auto outf = output_path(J);
		const path_t filefolder = (outf / full_name).string();
		if (!(std::filesystem::exists(filefolder))) {
			std::filesystem::create_directory(filefolder);
		}
		return filefolder;
	}

	template < size_t P, typename VecType >
	struct do_write_csv 
	{
		static void write_in_csv(std::ofstream& outFile, VecType& i) // or std::deque<std::vector<float>>::const_iterator
		{
			outFile << i[P-1] ;
			outFile << ',';
			do_write_csv<P - 1, VecType>::write_in_csv(outFile, i);
		}
	};

	template< typename VecType >
	struct do_write_csv<1, VecType>
	{
		static void write_in_csv(std::ofstream& outFile, VecType& i)
		{
			outFile << i[1] << std::endl;
		}
	};


	inline void open_csv(std::ofstream& outFile, const std::string& full_path, const std::string& header)
  {
    outFile.open(full_path);
    outFile << header << std::endl;
  }

  template < size_t P, typename DecType>
	void export_data(const DecType& data_out, std::ofstream& outFile)
  {
    if (P != data_out[0].size()) { std::cout << "Warning: size of saving vector different that defined, data might be missing." << std::endl; }

    for (auto i : data_out)
    {
      do_write_csv<P, typename DecType::value_type>::write_in_csv(outFile, i);
    }
  }

	template <typename DecType>
	void export_data(const DecType& data_out, std::ofstream& outFile)
	{
		for (auto i : data_out)
		{
			size_t p = i.size() - 1;
			while (!(i.empty()) && p != 0)
			{
				outFile << i[p] << ',';
				i.pop_back();
				--p;
			}
			outFile << i[0] << std::endl;
		}
	}


	template < size_t P, typename DecType>
	inline void export_csv(const DecType& data_out, const std::string& full_path, const std::string& header)
	{
		if (P != data_out[0].size()) { std::cout << "Warning: size of saving vector different that defined, data might be missing." << std::endl; }
		
		std::ofstream outFile;

		outFile.open(full_path);
		outFile << header << std::endl;
		for (auto i : data_out) 
		{
			do_write_csv<P, typename DecType::value_type>::write_in_csv(outFile, i);
		}
		outFile.close();
	}

	template < size_t P, typename DecType>
	inline void export_csv(const DecType& data_out, const std::string& full_path)
	{
		if (P != data_out[0].size()) { std::cout << "Warning: size of saving vector different that defined, data might be missing." << std::endl; }

		std::ofstream outFile;

		outFile.open(full_path, std::ios_base::app);
		for (auto i : data_out)
		{
			do_write_csv<P, typename DecType::value_type>::write_in_csv(outFile, i);
		}
		outFile.close();
	}
}
#endif
