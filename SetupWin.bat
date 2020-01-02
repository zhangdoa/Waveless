git submodule update

cd GitSubmodules

xcopy /y imgui\*.h ..\Source\ThirdParty\ImGui\
xcopy /y imgui\*.cpp ..\Source\ThirdParty\ImGui\

xcopy /y imgui\examples\imgui_impl_win32.h ..\Source\ThirdParty\ImGui\
xcopy /y imgui\examples\imgui_impl_win32.cpp ..\Source\ThirdParty\ImGui\

xcopy /y imgui\examples\imgui_impl_dx11.h ..\Source\ThirdParty\ImGui\
xcopy /y imgui\examples\imgui_impl_dx11.cpp ..\Source\ThirdParty\ImGui\

xcopy /y imgui\examples\imgui_impl_opengl3.h ..\Source\ThirdParty\ImGui\
xcopy /y imgui\examples\imgui_impl_opengl3.cpp ..\Source\ThirdParty\ImGui\

(echo #define IMGUI_IMPL_OPENGL_LOADER_GLAD) >temp.h.new
type ..\..\Source\ThirdParty\ImGui\imgui_impl_opengl3.h >>temp.h.new
move /y temp.h.new ..\..\Source\ThirdParty\ImGui\imgui_impl_opengl3.h

pause