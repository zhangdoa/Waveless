#pragma once

class WsEditor
{
public:
	WsEditor() = default;
	~WsEditor() = default;

	void Setup();
	void Initialize();
	void Render();
	void Terminate();
};