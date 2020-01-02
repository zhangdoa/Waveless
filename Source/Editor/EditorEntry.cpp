#pragma once
#include "Editor.h"

int main(int argc, char *argv[])
{
	WsEditor editor;
	editor.Setup();
	editor.Initialize();
	editor.Render();
	editor.Terminate();

	return 0;
}