file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
add_library(WsRuntime SHARED ${HEADERS} ${SOURCES})
set_property(TARGET WsRuntime PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(WsRuntime WsCore)
target_link_libraries(WsRuntime WsIO)