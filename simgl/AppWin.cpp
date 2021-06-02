#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdio.h>
#include <thread>
#include "AppWin.h"


AppWin::AppWin() : 
  param_{}
{
  game_throttle_.pause(true);    // start paused
  param_.sim.speedup = game_throttle_.speedup();
  CRect WindowExt;
  WindowExt.SetRect(256, 256, 256 + 1024 + param_.sim.textwidth96 - 4 * param_.sim.border, 256 + 1024);
  static_cast<CWindowImpl<AppWin>*>(this)->Create(NULL,
    &WindowExt,
    NULL,
    WS_OVERLAPPEDWINDOW,
    NULL);
  if (m_hWnd == NULL) throw std::runtime_error("Window creation failed");

  // DPI
  auto dpi = static_cast<int>(GetDpiForWindow(m_hWnd));
  auto dpiScale = double(dpi) / USER_DEFAULT_SCREEN_DPI;
  param_.sim.textwidth = static_cast<int>(dpiScale * param_.sim.textwidth96);
  const HINSTANCE hInstance = _AtlBaseModule.GetResourceInstance();
  const HICON hIcon = NULL;
  SetIcon(hIcon);
  SetWindowText(_T("Pigeon"));
}


AppWin::~AppWin()
{
  if (m_hWnd) 
  {
    BOOL dummy;
    OnDestroy(0, 0, 0, dummy);
    this->DestroyWindow();
  }
}


void AppWin::OnFinalMessage(HWND)
{
}


int AppWin::AppLoop(model::Simulation* sim, const json& J)
{
  MSG msg;
  BOOL bRet;
  sim_ = sim;
  retval_ = 0;
  renderer_.reset(new Renderer(glsl::Context(m_hWnd), J, param_));
  game_throttle_.transform_speedup([&](auto sp) { return double(J["Simulation"]["speedup"]); });
  game_throttle_.pause(false);
  ShowWindow(SW_NORMAL);
  while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
    if (bRet == -1) {
      break;
    }
    else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  sim_ = nullptr;
  return retval_;
}


void AppWin::notify(long long lmsg, const model::Simulation& sim)
{
  notify_next(lmsg, sim);
  using Msg = model::Simulation::Msg;
  auto msg = Msg(lmsg);
  switch (msg) {
    case Msg::Tick: {
      notify_tick(sim);
      break;
    }
    case Msg::Finished:
      finished_ = true;
      PostMessage(WM_CLOSE);
      break;
  }
}

void AppWin::send_flush_message(const model::Simulation& sim)
{
  SendMessage(WM_FLUSH_STATE, 0, reinterpret_cast<uintptr_t>(&sim));
  flush_ = false;
}


void AppWin::notify_tick(const model::Simulation& sim)
{
  send_flush_message(sim);
  game_throttle_.tick(std::chrono::duration<double>(model::Simulation::dt()), [&]() {
    if (flush_) {
      send_flush_message(sim);
    }
  });
}
 

// update renderer state and [render]
LRESULT AppWin::OnFlushState(UINT, WPARAM, LPARAM lpsim, BOOL&)
{
  auto sim = reinterpret_cast<const model::Simulation*>(lpsim);
  {
    std::lock_guard<std::mutex> _(appMutex_);
    renderer_->flush_state(*this, *sim);
  }
  renderer_->render();
  return 1;
}


LRESULT AppWin::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
{
  flush_ = true;
  return 1;
}


LRESULT AppWin::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
  return 0;
}


LRESULT AppWin::OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
{
  return 0;
}


LRESULT AppWin::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  auto W = GET_X_LPARAM(lParam);
  auto H = GET_Y_LPARAM(lParam);
  if (renderer_) renderer_->onSize(W, H);
  update_window();
  return 0;
}


LRESULT AppWin::OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
  auto mmi = (MINMAXINFO*)lParam;
  mmi->ptMinTrackSize = CPoint(500, 420);
  bHandled = TRUE;
  return 0;
}


LRESULT AppWin::OnKeyDown(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = true;
  bool shift = GetKeyState(VK_SHIFT) < 0;
  bool ctrl = GetKeyState(VK_CONTROL) < 0;
  bool alt = GetKeyState(VK_MENU) < 0;
  switch (wParam)
  {
  case VK_ESCAPE:
    game_throttle_.fast_forward(100);
    PostMessage(WM_CLOSE);
    bHandled = TRUE;
    break;

  case VK_SPACE:
    game_throttle_.toggle_pause();
    break;

  case VK_RETURN:
    if (shift) {
      retval_ = ctrl ? 2 : 1;
      game_throttle_.fast_forward(100);
      PostMessage(WM_CLOSE);
    }
    break;

  case VK_PRIOR: {
    param_.sim.speedup = game_throttle_.transform_speedup([](double su) { return 2.0 * su; });
    break;
  }

  case VK_NEXT: {
    param_.sim.speedup = game_throttle_.transform_speedup([](double su) { return std::max(1.0 / 64.0, 0.5 * su); });
    break;
  }

  case VK_HOME: {
    param_.sim.speedup = game_throttle_.transform_speedup([](double) { return 1.0; });
    break;
  }

  case VK_RIGHT:
    game_throttle_.fast_forward(1);
    break;

  case 'A':
    param_.sim.bkgDimm = (shift) ? 1.f : 0.25f;
    break;

  case 'S':
	  if (ctrl) {
      notify_once(*sim_); // NEED LOCK???
	  }
	  break;

  case 'K': {
    sim_->set_alive<model::pred_tag>(shift);
    break;
  }

  // petrubate individual (to React)
  case 'R': {
    //auto reng = rndutils::make_random_engine_low_entropy<>();
    //auto pidx = (std::uniform_int_distribution<size_t>(0, sim_->are_alive<model::pigeon_tag>()))(reng);

    //// csv writing backwards, so vectors backwards from header, new element to be added in front
    ////sim_->petrubate_alive<model::pigeon_tag>();
    //sim_->visit_all<model::pigeon_tag>([&](auto& p, size_t idx, bool alive) {
    //  if (p.idx == pidx)
    //  {
    //    p.stress_ = 1;
    //  }
    //});

    break;
  }

  case 'T': {
    auto species = std::min(shift ? 1ull : ctrl ? 2ull : alt ? 3ull : 0ull, model::n_species);
    param_.sim.trail[species] = !param_.sim.trail[species];
    break;
  }

  case 'P': {
    sim_->visit<model::pred_tag>([](auto& p) {
      std::cout << p.get_current_state() << ' ';
      });
    std::cout << std::endl;
    break;
  }

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8': {
    unsigned species = shift ? 1 : ctrl ? 2 : alt ? 3 : 0;
    renderer_->set_color_map(species, static_cast<unsigned>(wParam - '0'));
    break;
  }
  default:
    bHandled = renderer_->onKeyDown(static_cast<int>(wParam), shift, alt, ctrl);
    break;
  }
  flush_ = true;
  return 1;
}


LRESULT AppWin::OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  mouseX_ = GET_X_LPARAM(lParam);
  mouseY_ = GET_Y_LPARAM(lParam);
  if (renderer_->hit_test(mouseX_, mouseY_)) {
    SetCapture();
    tracking_ = true;
  }
  update_window();
  return 0;
}


LRESULT AppWin::OnLButtonUp(UINT, WPARAM, LPARAM, BOOL&)
{
  ReleaseCapture();
  return 0;
}


LRESULT AppWin::OnCaptureChanged(UINT, WPARAM, LPARAM, BOOL&)
{
  tracking_ = false;
  return 0;
}


LRESULT AppWin::OnLButtonDblClick(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  auto mouseX = GET_X_LPARAM(lParam);
  auto mouseY = GET_Y_LPARAM(lParam);
  auto nnr = renderer_->nearest_instance(mouseX, mouseY);
  if (nnr.pInstance) { 
    renderer_->follow(nnr.species, nnr.idx, nnr.dist2 > 1.f);
  }
  else {
    renderer_->follow(0, static_cast<size_t>(-1), false);
  }
  flush_ = true;
  return 0;
}


LRESULT AppWin::OnRButtonDblClick(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  auto mouseX = GET_X_LPARAM(lParam);
  auto mouseY = GET_Y_LPARAM(lParam);
  auto nnr = renderer_->nearest_instance(mouseX, mouseY);
  if (nnr.pInstance) {
    // hit test passed
    renderer_->follow(0, static_cast<size_t>(-1), false);
    flush_ = true;
  }
  return 0;
}


LRESULT AppWin::OnRButtonDown(UINT, WPARAM, LPARAM, BOOL&)
{
  return 0;
}


LRESULT AppWin::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  if (tracking_)
  {
    auto mouseX = GET_X_LPARAM(lParam);
    auto mouseY = GET_Y_LPARAM(lParam);
    renderer_->onMouseTrack(mouseX_ - mouseX, mouseY_ - mouseY);
    mouseX_ = mouseX;
    mouseY_ = mouseY;
    update_window();
  }
  return 0;
}


LRESULT AppWin::OnMouseWheel(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  auto mc = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
  ScreenToClient(&mc);
  int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
  renderer_->onMouseWheel(mc.x, mc.y, zDelta);
  update_window();
  return 0;
}


LRESULT AppWin::OnClose(UINT, WPARAM, LPARAM, BOOL&)
{
  sim_->terminate();
  if (!finished_) {
    PostMessage(WM_CLOSE);
    return 1;
  }
  renderer_.reset(nullptr);
  DestroyWindow();
  PostQuitMessage(0);
  return 0;
}


LRESULT AppWin::OnDpiChanged(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  auto dpi = static_cast<int>(LOWORD(wParam));
  if (renderer_) {
    renderer_->onDpiChanged(dpi);
  }
  SetWindowPos(NULL, (RECT*)lParam, NULL);
  return 0;
}


void AppWin::update_window()
{
  flush_ = true;
  InvalidateRect(NULL, TRUE);
  UpdateWindow();
}
