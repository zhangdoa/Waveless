#pragma once

class IImGuiWindow
{
public:
	ImGuiWindowWin() = default;
	virtual ~ImGuiWindowWin() = default;

	virtual bool setup() = 0;
	virtual bool initialize() = 0;
	virtual bool newFrame() = 0;
	virtual bool terminate() = 0;
};
