#pragma once
#include "IImGuiWindow.h"

class ImGuiWindowLinux : public IImGuiWindow
{
public:
	ImGuiWindowLinux() = default;
	~ImGuiWindowLinux() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool terminate() override;
};
