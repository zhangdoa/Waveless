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

include_directories("GitSubmodules/imgui-node-editor/NodeEditor/Include" "GitSubmodules/miniaudio")

add_subdirectory("Core")
add_subdirectory("IO")
add_subdirectory("Runtime")
#add_subdirectory("Editor")
add_subdirectory("Test")