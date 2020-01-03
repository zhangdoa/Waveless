cmake_minimum_required(VERSION 3.10.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (Waveless)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/LibArchive)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(WS_OS_WIN OFF)
option(WS_OS_LINUX OFF)
option(WS_OS_MAC OFF)
option(WS_RENDERER_DIRECTX OFF)
option(WS_RENDERER_OPENGL OFF)
option(WS_RENDERER_VULKAN OFF)
option(WS_RENDERER_METAL OFF)

if (WIN32)
set(WS_OS_WIN ON)
set(WS_RENDERER_DIRECTX ON)
endif (WIN32)

if (UNIX AND NOT APPLE)
set(WS_OS_LINUX ON)
endif (UNIX AND NOT APPLE)

if (APPLE)
set(WS_OS_MAC ON)
set(WS_RENDERER_METAL ON)
endif (APPLE)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

if (VULKAN_FOUND)
set(WS_RENDERER_VULKAN ON)
endif (VULKAN_FOUND)

if (OPENGL_FOUND AND NOT WS_OS_MAC)
set(WS_RENDERER_OPENGL ON)
endif (OPENGL_FOUND AND NOT WS_OS_MAC)

configure_file ("${CMAKE_SOURCE_DIR}/Core/Config.h.in" "${CMAKE_SOURCE_DIR}/Core/Config.h" @ONLY)

include_directories("../GitSubmodules/imgui-node-editor/ThirdParty/imgui" "../GitSubmodules/imgui-node-editor/ThirdParty/stb_image" "../GitSubmodules/imgui-node-editor/NodeEditor/Include" "../GitSubmodules/miniaudio")

add_subdirectory("Core")
add_subdirectory("IO")
add_subdirectory("Runtime")
add_subdirectory("Editor")
add_subdirectory("Test")