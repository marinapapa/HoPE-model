// 'compute shader' device
// Hanno 2019


#ifndef CSPERLIN_CSDEVICE_HPP_INCLUDED
#define CSPERLIN_CSDEVICE_HPP_INCLUDED

#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <memory>
#include <condition_variable>
#include <functional>
#include <glsl/wgl_context.hpp>


namespace cs {


  template <typename Fun, typename ...Args>
  using result_type_t = std::invoke_result_t<std::decay_t<Fun>, std::decay_t<Args>...>;

  
  void WaitForSync(GLsync);


  template <typename Ret>
  class compute_future
  {
  public:
    compute_future(compute_future&&) = default;
    compute_future& operator=(compute_future&&) = default;
    compute_future(const compute_future&) = delete;
    compute_future& operator=(const compute_future&) = delete;
    ~compute_future();

    Ret get();

  private:
    friend class csDevice;
    static auto make_promise() { return std::make_unique<std::promise<std::pair<GLsync, Ret>>>(); }

    compute_future(class csDevice* device, std::promise<std::pair<GLsync, Ret>>* promise) : 
      device_(device)
    { 
      future_ = promise->get_future();
    }
    csDevice* device_;
    std::future<std::pair<GLsync, Ret>> future_;
  };


  template <>
  class compute_future<void>
  {
  public:
    compute_future(compute_future&&) = default;
    compute_future& operator=(compute_future&&) = default;
    compute_future(const compute_future&) = delete;
    compute_future& operator=(const compute_future&) = delete;
    ~compute_future();

    void get();

  private:
    friend class csDevice;
    static auto make_promise() { return std::make_unique<std::promise<GLsync>>(); }

    compute_future(class csDevice* device, std::promise<GLsync>* promise) : 
      device_(device)
    {
      future_ = promise->get_future();
    }
    csDevice* device_;
    std::future<GLsync> future_;
  };


  class HeadlessContext
  {
  public:
    HeadlessContext();
    ~HeadlessContext();

    glsl::Context* glctx() { return glctx_.get(); }

  private:
    std::unique_ptr<class csDummyWin> dwnd_;
    std::unique_ptr<glsl::Context> glctx_;
  };


  class csDevice
  {
  public:
    // creates compute shader device on headless context
    explicit csDevice();

    // creates compute shader device on the given context
    // set shared==true if this context is used concurrently
    // (implies heavy performance penalty)
    explicit csDevice(glsl::Context* glCtx, bool shared);
    
    ~csDevice();

    template <class Fun, class... Args>
    auto compute(Fun&& f, Args&& ... args) -> compute_future<result_type_t<Fun, Args...>>;

  private:
    template <typename Ret> friend class compute_future;

    enum class Command {
      Wait = 0,
      Comp,
      Sync,
      KillSync,
      Quit,
      Initialize
    };

    template <typename Ret>
    struct do_compute {
      template <typename Fun, typename ...Args>
      static compute_future<Ret> apply(csDevice*, Fun&&, Args&& ...);
    };
      
    static void service_commands(csDevice* self, glsl::Context* glCtx, bool shared);
    void signal(Command) const;
    void signal(GLsync, Command) const;
    void signal(std::function<void(void)>&& fun) const;

    mutable std::mutex client_mutex_;     // synchronize compute thread
    mutable std::mutex cv_mutex_;         // synchronize command execution
    mutable std::condition_variable cv_;
    mutable Command cmd_ = Command::Initialize;
    mutable GLsync sync_cmd_state_ = nullptr;
    mutable std::function<void(void)> comp_cmd_state_;
    std::thread csthread_;
    std::unique_ptr<HeadlessContext> hlCtx_;    // optional out of blue context
  };


  template <typename Ret>
  template <typename Fun, typename ...Args>
  inline compute_future<Ret> csDevice::do_compute<Ret>::apply(csDevice* device, Fun&& fun, Args&& ...args)
  {
    auto promise = compute_future<Ret>::make_promise();
    auto cs = compute_future<Ret>(device, promise.get());
    device->signal([fun{std::move(fun)}, pp=promise.release()] () {
      try {
        auto result = fun(std::forward<Args>(args)...);
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        pp->set_value({sync, std::move(result)});
      }
      catch (...) {
        pp->set_exception(std::current_exception());
      }
      delete pp;
    });
    return std::move(cs);
  }


  template <> 
  template <typename Fun, typename ...Args>
  inline compute_future<void> csDevice::do_compute<void>::apply(csDevice* device, Fun&& fun, Args&& ...args)
  {
    auto promise = compute_future<void>::make_promise();
    auto cs = compute_future<void>(device, promise.get());
    device->signal([fun{std::move(fun)}, pp=promise.release()] () {
      try {
        fun(std::forward<Args>(args)...);
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        pp->set_value(sync);
      }
      catch (...) {
        pp->set_exception(std::current_exception());
      }
      delete pp;
    });
    return std::move(cs);
  }


  template<class Fun, class ...Args>
  inline auto csDevice::compute(Fun&& f, Args&& ...args) -> compute_future<result_type_t<Fun, Args...>>
  {
    return do_compute<result_type_t<Fun, Args...>>::apply(this, std::forward<Fun>(f), std::forward<Args>(args)...);
  }


  template <typename Ret>
  inline compute_future<Ret>::~compute_future()
  {
    if (future_.valid()) {
      device_->signal(future_.get().first, csDevice::Command::KillSync);
    }
  }


  template <typename Ret>
  inline Ret compute_future<Ret>::get()
  {
    auto sr = future_.get();
    device_->signal(sr.first, csDevice::Command::Sync);
    return std::move(sr.second);
  }


  inline compute_future<void>::~compute_future()
  {
    if (future_.valid()) {
      device_->signal(future_.get(), csDevice::Command::KillSync);
    }
  }


  inline void compute_future<void>::get()
  {
    device_->signal(future_.get(), csDevice::Command::Sync);
  }

}

#endif
