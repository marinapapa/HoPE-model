#ifndef APPWIN_H_INCLUDED
#define APPWIN_H_INCLUDED

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#ifdef __INTEL_COMPILER
#define ATL
(expr)   // avoid 'too many categories' assertion
#endif

#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <string>
#include <future>
#include <mutex>
#include <set>
#include <game_watches.hpp>
#include "model/simulation.hpp"
#include "agents/agents.hpp"
#include "Renderer.h"
#include "observer.hpp"


#define WM_FLUSH_STATE (WM_USER + 1)


class AppWin :
  public CWindowImpl<AppWin>,
  public model::Observer
{
public:
  AppWin();
  virtual ~AppWin();
  int AppLoop(model::Simulation* sim, const json& J);

  DECLARE_WND_CLASS_EX(0, CS_OWNDC | CS_DBLCLKS, NULL);

  BEGIN_MSG_MAP(AppWin)
    MESSAGE_HANDLER(WM_FLUSH_STATE, OnFlushState);
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd);
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown);
    MESSAGE_HANDLER(WM_SIZE, OnSize);
    MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo);
    MESSAGE_HANDLER(WM_CREATE, OnCreate);
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown);
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp);
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick);
    MESSAGE_HANDLER(WM_RBUTTONDBLCLK, OnRButtonDblClick);
    MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown);
    MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged);
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
    MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel);
    MESSAGE_HANDLER(WM_CLOSE, OnClose);
    MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged);
  END_MSG_MAP()

  LRESULT OnFlushState(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnGetMinMaxInfo(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnLButtonDown(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnLButtonUp(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnLButtonDblClick(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnRButtonDblClick(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnRButtonDown(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnCaptureChanged(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnMouseWheel(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnDpiChanged(UINT, WPARAM, LPARAM, BOOL&);
  void OnFinalMessage(HWND hWnd) override;

  // Observer interface
  void notify(long long lmsg, const class model::Simulation& sim) override;

  // access for renderer
  const auto& sim_param() const { return param_.sim; }

private:
  void notify_tick(const model::Simulation& sim);
  void update_window();
  void send_flush_message(const class model::Simulation& psim);

  game_watches::game_throttle<> game_throttle_;
  bool tracking_ = false;
  int mouseX_ = 0;
  int mouseY_ = 0;

  // interaction with model thread
  mutable std::mutex appMutex_;
  std::atomic<bool> flush_ = false;
  std::atomic<bool> finished_ = false;
  bool perf_ = false;

  std::unique_ptr<class Renderer> renderer_;  // externally synchronized!
  model::Simulation* sim_ = nullptr;
  int retval_ = 0;

  Param param_;
  Observer* called_obs_ = nullptr;
};


#endif
