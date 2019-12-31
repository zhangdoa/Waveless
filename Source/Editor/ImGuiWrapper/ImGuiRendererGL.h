#pragma once
#include "IImGuiRenderer.h"

class ImGuiRendererGL : public IImGuiRenderer
{
public:
	ImGuiRendererGL() = default;
	~ImGuiRendererGL() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool render() override;
	bool terminate() override;
};