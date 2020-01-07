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

	void* LoadTexture(const char* path) override;
	void* CreateTexture(const void* data, int width, int height) override;
	void DestroyTexture(void* texture) override;
	int GetTextureWidth(void* texture) override;
	int GetTextureHeight(void* texture) override;

	static bool resize(unsigned int x, unsigned int y);
};