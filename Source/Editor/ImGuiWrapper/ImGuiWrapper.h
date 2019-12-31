#pragma once

class ImGuiWrapper
{
public:
	~ImGuiWrapper() {};

	static ImGuiWrapper& get()
	{
		static ImGuiWrapper instance;
		return instance;
	}
	bool Setup();
	bool Initialize();
	bool Render();
	bool Terminate();

private:
	ImGuiWrapper() {};
};