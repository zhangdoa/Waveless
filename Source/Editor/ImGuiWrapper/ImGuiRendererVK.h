#pragma once
#include "IImGuiRenderer.h"

class ImGuiRendererVK : public IImGuiRenderer
{
public:
	ImGuiRendererVK() = default;
	~ImGuiRendererVK() = default;

	bool setup() override;
	bool initialize() override;
	bool newFrame() override;
	bool render() override;
	bool terminate() override;
};