cd GitSubmodules/imgui-node-editor
mkdir Build
cd Build
cmake -G "Visual Studio 15 Win64" ../
msbuild ImGuiNodeEditor.sln
xcopy /s/e/y NodeEditor\Debug\*.lib ..\..\..\Build\LibArchive\Debug
pause