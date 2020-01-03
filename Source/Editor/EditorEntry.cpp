#pragma once
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Editor.h"

int main(int argc, char *argv[])
{
# if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
# endif
	WsEditor editor;
	editor.Setup();
	editor.Initialize();
	editor.Render();
	editor.Terminate();

	return 0;
}