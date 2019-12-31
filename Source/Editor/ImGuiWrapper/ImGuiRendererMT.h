#pragma once
#include "IImGuiRenderer.h"

class ImGuiRendererMT : public IImGuiRenderer
{
public:
	ImGuiRendererMT() = default;
	~ImGuiRendererMT() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool render() override;
	bool terminate() override;
};