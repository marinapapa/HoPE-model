#ifndef ANALYSIS_OBS_HPP_INCLUDED
#define ANALYSIS_OBS_HPP_INCLUDED

#include <analysis/analysis.hpp>

namespace analysis
{
	template <typename Tag>
	class TimeSeriesObserver : public model::AnalysisObserver
	{
	public:
		TimeSeriesObserver(const std::filesystem::path& out_path, const json& J)
			: AnalysisObserver(out_path, J)
		{
			analysis::open_csv(outfile_stream_, full_out_path_, header_);
		}
		~TimeSeriesObserver() override {}

		void notify_collect(const model::Simulation& sim) override
		{
			const auto tt = static_cast<float>(sim.tick()) * model::Simulation::dt();

			sim.visit_all<Tag>([&](auto& p, size_t idx, bool alive) {
				// csv writing backwards, so vectors backwards from header, new element to be added in front
				if (alive) {
					const auto& nv = sim.sorted_view<Tag, pred_tag>(idx); // predators
					//const auto& fi = sim.flocks<Tag>();										// all flocks
					const auto fl_id = sim.flock_of<Tag>(idx);
					const auto& thisflock = sim.flocks<Tag>()[fl_id];
					const auto dist2cent = torus::distance(Simulation::WH(), p.pos, thisflock.gc()); // distance to center of flock
					const auto dir2fcent = glm::normalize(torus::ofs(Simulation::WH(), p.pos, thisflock.gc()));
					const auto head_dev = glm::degrees(math::rad_between(p.dir, thisflock.vel));		 // deviation of self heading to flocks heading
					const auto centr = centrality(p, idx, sim);
					const auto rad2fcent = math::rad_between(p.dir, dir2fcent);
					auto confl = -1.f; // confict scenario -1 if no predator present
					auto dist2pred = -1.f; // if no predator present
					auto radAwayPred = -1.f; // if no predator present
					vec_t dir2pred(-1.f, -1.f);
					if (nv.size())
					{
						const auto& predator = sim.pop<pred_tag>()[nv[0].idx];    // nearest predator
						dist2pred = torus::distance(Simulation::WH(), p.pos, predator.pos);
						confl = in_conflict_dir_ali(p, predator, thisflock);
						dir2pred = glm::normalize(torus::ofs(Simulation::WH(), p.pos, predator.pos));
						radAwayPred = math::rad_between(predator.dir, p.dir);
					}
				  const auto nn = sim.sorted_view<Tag>(idx).cbegin(); // nearest neighbor
				  data_out_.push_back({ confl, dir2pred.y, dir2pred.x, dist2pred, radAwayPred, dir2fcent.y, dir2fcent.x, rad2fcent, dist2cent, head_dev, static_cast<float>(fl_id), static_cast<float>(p.get_current_state()), centr, p.ang_vel, p.accel.y, p.accel.x, p.speed, p.dir.y,  p.dir.x,  p.pos.y, p.pos.x, static_cast<float>(idx), tt });
				}
				//data_out_.push_back({});
			});
		}

		void notify_save(const model::Simulation& sim) override
		{
			if (data_out_.empty()) { return; }
			std::cout << "Saving timeseries data.." << std::endl;
			analysis::export_data(data_out_, outfile_stream_);
		}

	private:
		const std::string header_ = "time,id,posx,posy,dirx,diry,speed,accelx,accely,ang_vel,centr,state,f_id,diff_head,dist2fcent,rad2fcent,dirX2fcent,dirY2fcent,radAwayPred,dist2pred,dirX2pred,dirY2pred,conflict";
	};


	template <typename Tag>
	class FlockObserver : public model::AnalysisObserver
	{
	public:
		FlockObserver(const std::filesystem::path & out_path, const json& J)
			: AnalysisObserver(out_path, J)
		{
			analysis::open_csv(outfile_stream_, full_out_path_, header_);
		}
		~FlockObserver() override {}

		void notify_collect(const model::Simulation& sim) override
		{
			const auto& fi = sim.flocks<Tag>();
			size_t idx = 0;
			const auto tt = static_cast<float>(sim.tick())* model::Simulation::dt();
			// csv writing backwards, so vectors backwards from header, new element to be added in front
			for (auto& i : fi)
			{
        //auto tc = analysis::turn_correl(i, sim);
				data_out_.push_back({ i.H[1].y, i.H[1].x, i.H[0].y, i.H[0].x, i.ext.y, i.ext.x, i.H[2].y, i.H[2].x, i.vel.y, i.vel.x, static_cast<float>(i.size), static_cast<float>(idx), tt });
				++idx;    
			}
		}

		void notify_save(const model::Simulation& sim) override
		{ 
			if (data_out_.empty()) { return;	}

			if (n_param_ > data_out_[0].size()) {
				std::cout << "Warning: size of saving vector lower than defined, data wont be saved." << std::endl;
				return;
			}
			std::cout << "Saving flock data.." << std::endl;
			analysis::export_data(data_out_, outfile_stream_);
		}

	private:
		static const size_t n_param_ = 13; // number of parameters to save
		const std::string header_ = "time,id,size,velx,vely,fcX,fcY,obbExtX,obbExtY,obbH0X,obbH0Y,obbH1X,obbH1Y";
	};


	template <typename Tag>
	class SnapShotObserver : public model::Observer
	{
	public:
		SnapShotObserver(const std::filesystem::path & out_path, const json& J)
		{
			const std::string out_name = J["output_name"];
			full_out_path_ = out_path / out_name;
			n_ = 0;
		}
		~SnapShotObserver() override {}

		void notify_once(const model::Simulation& sim) override
		{
		  notify_collect(sim);

	      if (data_out_.empty()) { return; }

		  const auto filepath = full_out_path_.string() + "_" + std::to_string(n_) + ".csv";
		  const std::string header = "id,posx,posy,dirx,diry,speed,accelx,accely";
		  analysis::open_csv(outfile_stream_, filepath, header);
		  notify_save(sim, outfile_stream_);
		  outfile_stream_.close();
		}

		void notify_collect(const model::Simulation& sim)
		{
		  sim.visit_all<Tag>([&](auto& p, size_t idx, bool alive) {
			// csv writing backwards, so vectors backwards from header, new element to be added in front
			if (alive) 
			{
				data_out_.push_back({ p.accel.y, p.accel.x, p.speed, p.dir.y,  p.dir.x,  p.pos.y, p.pos.x, static_cast<float>(idx) });
			}
		  });
		}

		void notify_save(const model::Simulation& sim, std::ofstream& outFile)
		{
			if (n_param_ > data_out_[0].size()) {
				std::cout << "Warning: size of saving vector lower than defined, data wont be saved." << std::endl;
				data_out_.clear();
				return;
			}
			std::cout << "Taking data snapshot.." << std::endl;
	  		analysis::export_data<n_param_>(data_out_, outFile);
			outFile.close();
			++n_;
			data_out_.clear();
		}

	private:
		std::deque<std::array<float, 9>> data_out_; // idx, pos.x, pos.y, dir.x, dir.y, speed, accel.x, accel.y
		static const size_t n_param_ = 9; // number of parameters to save in timeseries
		std::filesystem::path full_out_path_;
		size_t n_; // number of snapshots taken
		std::ofstream outfile_stream_;
	};


	class DataExpObserver : public model::Observer
	{
	public:
		DataExpObserver(const json& J)
		{
		  auto& ja = J["Simulation"]["Analysis"];

			// what to plot
			const auto& jo = ja["Observers"];
			const std::string outf = ja["output_path"];

			json_ext_ = ja["Externals"];
			json_ext_["output_path"] = outf;

			const std::string confname = J["Simulation"]["Analysis"]["Externals"]["configName"];
			std::ofstream config_file_id(outf + "/" + confname + ".txt"); // create config id as name of empty file in folder

			save_json(J, (path_t(outf) / "composed_config.json"));
		}
		~DataExpObserver() {}

		void notify(long long lmsg, const model::Simulation& sim)
		{
			using Msg = model::Simulation::Msg;
			auto msg = Msg(lmsg);

			switch (msg) {
			case Msg::Finished:
				notify_save(sim);
				break;
			default:
			  break;
			}
			//notify_next(lmsg, sim);
		}

		void notify_save(const model::Simulation& sim)
		{
		}

  private:
		json json_ext_;
  };


	template <typename Tag>
	class AllNeighborsObserver : public model::AnalysisObserver
	{
	public:
		AllNeighborsObserver(const std::filesystem::path& out_path, const json& J, const size_t& N)
			: AnalysisObserver(out_path, J)
		{	
			std::string header = "time,id,flock_id";
			for (auto i = 1; i < N; ++i)
			{
				header += ",idOfn" + std::to_string(i);
				header += ",dist2n" + std::to_string(i);
				header += ",bAngl2n" + std::to_string(i);
				header += ",dirX2n" + std::to_string(i);
				header += ",dirY2n" + std::to_string(i);
			}
			analysis::open_csv(outfile_stream_, full_out_path_, header);
		}

		~AllNeighborsObserver() override {}

		void notify_collect(const model::Simulation& sim) override
		{
			const auto& flock = sim.pop<Tag>();
			const auto tt = static_cast<float>(sim.tick())* model::Simulation::dt();

			sim.visit_all<Tag>([&](auto& p, size_t idx, bool alive) {
				// csv writing backwards, so vectors backwards from header, new element to be added in front
				if (alive) {

					data_out_.push_back(std::vector<float>());
					const auto& all_nb = sim.sorted_view<Tag>(idx); // all neighbors

					if (all_nb.size())
					{
						for (auto it = all_nb.cend() - 1; it != all_nb.cbegin() - 1; --it) { // reverting iterator cause of csv saving function
							auto dir2 = math::save_normalize(torus::ofs(sim.WH(), p.pos, flock[it->idx].pos), vec_t(0.f));
							data_out_.back().insert(data_out_.back().end(), { dir2.y, dir2.x, it->bangl, std::sqrt(it->dist2), static_cast<float>(it->idx) });
						}
					}
					data_out_.back().insert(data_out_.back().end(), { static_cast<float>(sim.flock_of<Tag>(idx)), static_cast<float>(idx), tt });
				}
			});
		}

		void notify_save(const model::Simulation& sim) override
		{
			if (data_out_.empty()) { return; }
			std::cout << "Saving neighbors data.." << std::endl;
			analysis::export_data(data_out_, outfile_stream_);
		}
	};


	template <typename Tag>
	class ForcesObserver : public model::AnalysisObserver
	{
	public:
		ForcesObserver(const std::filesystem::path& out_path, const json& J)
			: AnalysisObserver(out_path, J)
		{
			analysis::open_csv(outfile_stream_, full_out_path_, header_);
		}
		~ForcesObserver() override {}

		void notify_collect(const model::Simulation& sim) override
		{
			const auto tt = static_cast<float>(sim.tick())* model::Simulation::dt();

			sim.visit_all<Tag>([&](auto& p, size_t idx, bool alive) {
				// csv writing backwards, so vectors backwards from header, new element to be added in front
				if (alive) {
					data_out_.push_back({ p.f_sep_ang , p.f_coh_ang, p.f_ali_ang, static_cast<float>(idx), tt });
				}
				});
		}

		void notify_save(const model::Simulation& sim) override
		{
			if (data_out_.empty()) { return; }
			std::cout << "Saving forces data.." << std::endl;
			analysis::export_data(data_out_, outfile_stream_);
		}

	private:
		const std::string header_ = "time,id,ali_angl,coh_angl,sep_angl";
	};


	template <typename Tag>
	std::vector<std::unique_ptr<Observer>> CreateObserverChain(json& J)
	{
		auto& ja = J["Simulation"]["Analysis"];
		auto& jpigeon = J["Pigeon"]["states"];
		auto& N = J["Pigeon"]["N"];
		std::vector<std::unique_ptr<Observer>> res;

		if (ja.size() == 0 || ja["data_folder"] == "")
		{
			std::cout << "No analysis observers created, data extraction will not take place." << std::endl; 
			return res; // no observers created
		}
		const auto unique_path = analysis::unique_output_folder(ja);

		// inject output path to json object
		ja["output_path"] = unique_path.string();

		const auto& jo = ja["Observers"];
		for (const auto& j : jo)
		{
			std::string type = j["type"];
			if (type == "TimeSeries") res.emplace_back(std::make_unique<TimeSeriesObserver<Tag>>(unique_path, j));
			else if (type == "FlockData") res.emplace_back(std::make_unique<FlockObserver<Tag>>(unique_path, j));
			else if (type == "NeighbData") res.emplace_back(std::make_unique<AllNeighborsObserver<Tag>>(unique_path, j, N));
			else if (type == "SnapShot") res.emplace_back(std::make_unique<SnapShotObserver<Tag>>(unique_path, j));
			else if (type == "CoordForces") res.emplace_back(std::make_unique<ForcesObserver<Tag>>(unique_path, j));
			else throw std::runtime_error("unknown observer");
		}
		res.emplace_back(std::make_unique<DataExpObserver>(J)); // has to be at the end of the chain
		return res;
	}
}

#endif
