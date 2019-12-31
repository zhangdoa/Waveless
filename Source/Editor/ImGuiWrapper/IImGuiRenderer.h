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
};
