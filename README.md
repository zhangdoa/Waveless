# Waveless
[![CodeFactor](https://www.codefactor.io/repository/github/zhangdoa/waveless/badge)](https://www.codefactor.io/repository/github/zhangdoa/waveless)
[![GPL-3.0 licensed](https://img.shields.io/badge/license-GPL--3.0-brightgreen.svg)](LICENSE.md)

An audio facility library targeting offline DSP rendering and real-time event-driven 3D audio framework.

## How to build
- Windows
1. Run `Script/SetupWin.bat`
2. Run `Script/BuildNodeEditorWin-VS15.bat`
3. Run `Script/BuildWin-VS15.bat`

## How to use
- Windows
1. Open WsEditor.exe
2. Create node graph on the canvas, and then save and compile it within the editor
3. Link your application against the generated WsCanvas.dll, and include the API headers

### Example
```cpp
#include "../Core/Math.h"
#include "../Runtime/AudioEngine.h"
#include "../Core/WsCanvasAPIExport.h"
WS_CANVAS_API void EventScript_MinimalTest(Vector& in_Position);

int main()
{
	AudioEngine::Initialize();
	Vector in_Position;

	EventScript_MinimalTest(in_Position);

	AudioEngine::Flush();
	AudioEngine::Terminate();
}
```
## Reference Materials

[EBU Tech 3285: BWF - A FORMAT FOR AUDIO DATA FILES IN BROADCASTING](https://tech.ebu.ch/publications/tech3285)

[EBU Tech 3306: RF64: AN EXTENDED FILE FORMAT FOR AUDIO](https://tech.ebu.ch/publications/tech3306)

[EBU Technical Recommendation R98-1999 Format for the <CodingHistory> field in Broadcast Wave Format files, BWF](https://tech.ebu.ch/docs/r/r098.pdf)

[The Next-Gen Dynamic Sound System of Killzone Shadow Fall](https://www.guerrilla-games.com/read/the-next-gen-dynamic-sound-system-of-killzone-shadow-fall)

## Third-party dependencies

[imgui](https://github.com/ocornut/imgui)

[imgui-node-editor](https://github.com/thedmd/imgui-node-editor)

[json](https://github.com/nlohmann/json)

[miniaudio](https://github.com/dr-soft/miniaudio)

