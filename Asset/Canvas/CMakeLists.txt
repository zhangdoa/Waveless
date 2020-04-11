cmake_minimum_required(VERSION 3.10.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Waveless-Canvas)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/LibArchive)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/../../Build/LibArchive/${CMAKE_BUILD_TYPE})

file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
add_library(WsCanvas SHARED ${HEADERS} ${SOURCES})

find_library(AudioEngine WsRuntime)
find_library(IO WsIO)

target_link_libraries(WsCanvas "${AudioEngine}")
target_link_libraries(WsCanvas "${IO}")