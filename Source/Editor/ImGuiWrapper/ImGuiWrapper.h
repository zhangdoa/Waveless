#pragma once
#include "../../Core/Typedef.h"

namespace Waveless
{
	class ImGuiWrapper
	{
	public:
		~ImGuiWrapper() {};

		static ImGuiWrapper& get()
		{
			static ImGuiWrapper instance;
			return instance;
		}
		WsResult Setup();
		WsResult Initialize();
		WsResult Render();
		WsResult Terminate();

	private:
		ImGuiWrapper() {};
	};
}