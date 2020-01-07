#pragma once

class IImGuiRenderer
{
public:
	IImGuiRenderer() = default;
	virtual ~IImGuiRenderer() = default;

	virtual bool setup() = 0;
	virtual bool initialize() = 0;
	virtual bool newFrame() = 0;
	virtual bool render() = 0;
	virtual bool terminate() = 0;

	virtual void* LoadTexture(const char* path) = 0;
	virtual void* CreateTexture(const void* data, int width, int height) = 0;
	virtual void DestroyTexture(void* texture) = 0;
	virtual int GetTextureWidth(void* texture) = 0;
	virtual int GetTextureHeight(void* texture) = 0;
};
