mkdir Build
cd Build
cmake -DCMAKE_BUILD_TYPE=DEBUG -G "Visual Studio 15 Win64" ../source
cmake -DCMAKE_BUILD_TYPE=DEBUG -G "Visual Studio 15 Win64" ../source
"%VS2017INSTALLDIR%\MSBuild\15.0\Bin\msbuild.exe" Waveless.sln
pause