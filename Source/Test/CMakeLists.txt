file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
add_executable(WsTest ${HEADERS} ${SOURCES})

target_link_libraries(WsTest WsCore)
target_link_libraries(WsTest WsIO)
target_link_libraries(WsTest WsRuntime)
target_link_libraries(WsTest ${CMAKE_SOURCE_DIR}/../Build-Canvas/LibArchive/${CMAKE_BUILD_TYPE}/WsCanvas.lib)