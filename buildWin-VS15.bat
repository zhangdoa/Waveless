mkdir build
cd build
cmake -G "Visual Studio 15 Win64" ../source
cmake -G "Visual Studio 15 Win64" ../source
msbuild Waveless.sln
pause