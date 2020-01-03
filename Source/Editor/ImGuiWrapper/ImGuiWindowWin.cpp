#include "ImGuiWindowWin.h"
#include "ImGuiRendererDX11.h"

#include "../../Core/stdafx.h"

#include "../../GitSubmodules/imgui-node-editor/Examples/Common/Application/Source/DX11/imgui_impl_win32.cpp"

namespace ImGuiWindowWinNS
{
	HINSTANCE m_hInstance;
	LPCSTR m_applicationName;
	HWND m_hwnd;
	HDC m_HDC;
	IImGuiRenderer* renderer;
}

using namespace ImGuiWindowWinNS;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI ImGui_WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			ImGuiRendererDX11::resize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool ImGuiWindowWin::setup()
{
	const auto c_ClassName = _T("WsEditor");
	const std::string c_WindowName = "WsEditor";

	// Create application window
	const auto wc = WNDCLASSEX
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		ImGui_WinProc,
		0L,
		0L,
		GetModuleHandle(nullptr),
		NULL,
		NULL,
		NULL,
		NULL,
		c_ClassName,
		NULL
	};

	RegisterClassEx(&wc);

	m_hwnd = CreateWindow(c_ClassName, c_WindowName.c_str(), WS_OVERLAPPEDWINDOW, 1920 + 100, 100, 1440, 800, nullptr, nullptr, wc.hInstance, nullptr);

	return true;
}

bool ImGuiWindowWin::initialize()
{
	ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);
	UpdateWindow(m_hwnd);

	ImGui_ImplWin32_Init(m_hwnd);

	return true;
}

bool ImGuiWindowWin::update()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_KEYDOWN && (msg.wParam == VK_ESCAPE))
		{
			PostQuitMessage(0);
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (IsIconic(m_hwnd))
	{
		return false;
	}

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

HWND ImGuiWindowWin::getHWND()
{
	return m_hwnd;
}