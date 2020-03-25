mkdir Build-Canvas
cd Build-Canvas
cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 15 Win64" ../Asset/Canvas
"%VS2017INSTALLDIR%\MSBuild\15.0\Bin\msbuild.exe" Waveless-Canvas.sln
pause