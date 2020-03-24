cd GitSubmodules/imgui-node-editor
mkdir Build
cd Build
cmake -G "Visual Studio 15 Win64" ../
"%VS2017INSTALLDIR%\MSBuild\15.0\Bin\msbuild.exe" ImGuiNodeEditor.sln
xcopy /s/e/y NodeEditor\Debug\*.lib ..\..\..\Build\LibArchive\Debug\
xcopy /s/e/y ThirdParty\imgui\Debug\*.lib ..\..\..\Build\LibArchive\Debug\
pause