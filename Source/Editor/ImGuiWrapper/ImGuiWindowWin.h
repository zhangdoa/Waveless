#pragma once
#include "IImGuiWindow.h"

class ImGuiWindowWin : public IImGuiWindow
{
public:
	ImGuiWindowWin() = default;
	~ImGuiWindowWin() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool terminate() override;
};