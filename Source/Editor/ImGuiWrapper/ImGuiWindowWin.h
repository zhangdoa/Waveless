#pragma once
#include "IImGuiWindow.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <windowsx.h>

class ImGuiWindowWin : public IImGuiWindow
{
public:
	ImGuiWindowWin() = default;
	~ImGuiWindowWin() = default;

	bool setup() override;
	bool initialize() override;
	bool update() override;
	bool newFrame() override;
	bool terminate() override;

	static HWND getHWND();
};