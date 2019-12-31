#include "ImGuiWindowWin.h"

#include "../ThirdParty/ImGui/imgui_impl_win32.cpp"

namespace ImGuiWindowWinNS
{
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ImGuiWindowWin::setup()
{
	return true;
}

bool ImGuiWindowWin::initialize()
{
	ImGui_ImplWin32_Init(WinWindowSystemComponent::get().m_hwnd);

	return true;
}

bool ImGuiWindowWin::newFrame()
{
	ImGui_ImplWin32_NewFrame();
	return true;
}

bool ImGuiWindowWin::terminate()
{
	ImGui_ImplWin32_Shutdown();

	return true;
}