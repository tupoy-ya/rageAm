#include "window.h"

#ifdef AM_STANDALONE
#include "window_standalone.h"
#else
#include "window_integrated.h"
#endif

amUniquePtr<rageam::Window> rageam::WindowFactory::sm_Window;

void rageam::Window::Show()
{
	ShowWindow(m_Handle, SW_SHOWDEFAULT);
	UpdateWindow(m_Handle);
}

void rageam::WindowFactory::CreateRenderWindow()
{
#ifdef AM_STANDALONE
	sm_Window = std::make_unique<WindowStandalone>();
#else
	sm_Window = std::make_unique<WindowIntegrated>();
#endif
	sm_Window->Show();
}

void rageam::WindowFactory::DestroyRenderWindow()
{
	sm_Window.reset();
}
