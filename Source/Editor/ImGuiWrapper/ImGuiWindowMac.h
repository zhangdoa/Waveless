#pragma once
#include "IImGuiWindow.h"

class ImGuiWindowMac : public IImGuiWindow
{
public:
	ImGuiWindowMac() = default;
	~ImGuiWindowMac() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool terminate() override;
};
