#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <glsl/debug.h>
#include "csDevice.hpp"


namespace cs {


  class csDummyWin :
    public CWindowImpl<csDummyWin>
  {
  public:
    DECLARE_WND_CLASS_EX(0, CS_OWNDC, NULL);

    BEGIN_MSG_MAP(csDummyWin)
    END_MSG_MAP()

    explicit csDummyWin();
    virtual ~csDummyWin();
  };


  csDummyWin::csDummyWin()
  {
    static_cast<CWindowImpl<csDummyWin>*>(this)->Create(
      NULL,
      NULL,
      NULL,
      WS_OVERLAPPEDWINDOW,
      NULL);
    if (NULL == m_hWnd) throw std::runtime_error("csDummyWin creation failed");
  };

    
  csDummyWin::~csDummyWin()
  {
    if (m_hWnd) DestroyWindow();
  }


  void WaitForSync(GLsync sync)
  {
    if (!glIsSync(sync)) return;
    GLenum ws = ws = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    while (!(ws == GL_ALREADY_SIGNALED || ws == GL_CONDITION_SATISFIED)) {
      ws = glClientWaitSync(sync, 0, 10000000);
      if (ws == GL_ALREADY_SIGNALED || ws == GL_CONDITION_SATISFIED) break;
      if (ws == GL_WAIT_FAILED) throw std::runtime_error("glClientWaitSync failed");
    }
  }


  HeadlessContext::HeadlessContext() :
    dwnd_(new csDummyWin())
  {
    glctx_.reset(new glsl::Context(dwnd_->m_hWnd));
    glctx_->ReleaseCurrent();
  }


  HeadlessContext::~HeadlessContext()
  {
    glctx_.reset(nullptr);
    dwnd_.reset(nullptr);
  }
  

  csDevice::csDevice()
  {
    hlCtx_.reset(new HeadlessContext());
    csthread_ = std::thread(&service_commands, this, hlCtx_->glctx(), false);
    // wait until csthread_ is ready to serve commands
    std::unique_lock<std::mutex> lock(cv_mutex_);
    cv_.wait(lock, [this] { return (Command::Wait == cmd_); });
  }


  csDevice::csDevice(glsl::Context* glCtx, bool shared)
  {
    csthread_ = std::thread(&service_commands, this, glCtx, shared);
    // wait until csthread_ is ready to serve commands
    std::unique_lock<std::mutex> lock(cv_mutex_);
    cv_.wait(lock, [this] { return (Command::Wait == cmd_); });
  }


  csDevice::~csDevice()
  {
    if (csthread_.joinable()) {
      signal(Command::Quit);
      csthread_.join();
    }
  }


  void csDevice::service_commands(csDevice* self, glsl::Context* glCtx, bool shared)
  {
    try {
      bool quit = false;
      while (!quit) {
        std::unique_lock<std::mutex> lock(self->cv_mutex_);
        self->cv_.wait(lock, [self] { return Command::Wait != self->cmd_; });
        if (self->cmd_ == Command::Comp) {
          auto fun = std::move(self->comp_cmd_state_);
          self->cmd_ = Command::Wait;
          self->cv_.notify_one();   // signal assignment
          if (shared) glCtx->MakeCurrent();
          fun();
        }
        else {
          if (shared) glCtx->MakeCurrent();
          switch (self->cmd_) {
          case Command::Sync:
            WaitForSync(self->sync_cmd_state_);
            // don't break, sync object must be destroyed
          case Command::KillSync:
            glDeleteSync(self->sync_cmd_state_);
            self->sync_cmd_state_ = nullptr;
            break;
          case Command::Quit:
            quit = true;
            break;
          case Command::Initialize:
            if (!shared) glCtx->MakeCurrent();   // once
#ifdef GLSL_DEBUG_OUTPUT
#ifdef GL_DEBUG_OUTPUT
            glEnable(GL_DEBUG_OUTPUT);
#endif
            glsl::SetDebugCallback(glsl::GLSL_DEBUG_MSG_LEVEL::LOW, glsl::GLDebugLogOnceStdErr);
#endif
            break;
          default:
            break;
          }
          self->cmd_ = Command::Wait;
          self->cv_.notify_one();   // signal completion
        }
        if (shared) glCtx->ReleaseCurrent();
      }
      return;
    }
    catch (std::exception& err) {
      std::cerr << err.what() << '\n';
    }
    catch (const char* what) {
      std::cerr << what << '\n';
    }
    catch (...) {
      std::cerr << "Unknown exception\n";
    }
    std::terminate(); 
  }


#define csDevice_signal_prolog() \
  std::lock_guard<std::mutex> _(client_mutex_); \
  std::unique_lock<std::mutex> lock(cv_mutex_)


#define csDevice_signal_epilog() \
  cv_.notify_one(); \
  cv_.wait(lock, [this] { return (Command::Wait == cmd_); })


  void csDevice::signal(Command cmd) const
  {
    csDevice_signal_prolog();
    cmd_ = cmd;
    csDevice_signal_epilog();
  }


  void csDevice::signal(GLsync sync, Command cmd) const
  {
    csDevice_signal_prolog();
    cmd_ = cmd;
    sync_cmd_state_ = sync;
    csDevice_signal_epilog();
  }


  void csDevice::signal(std::function<void(void)>&& fun) const
  {
    csDevice_signal_prolog();
    cmd_ = Command::Comp;
    comp_cmd_state_ = std::move(fun);
    csDevice_signal_epilog();
  }

#undef csDevice_signal_prolog
#undef csDevice_signal_epilog

}
