#pragma once
#include "IImGuiRenderer.h"

class ImGuiRendererDX11 : public IImGuiRenderer
{
public:
	ImGuiRendererDX11() = default;
	~ImGuiRendererDX11() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool render() override;
	bool terminate() override;

	static bool resize(unsigned int x, unsigned int y);
};