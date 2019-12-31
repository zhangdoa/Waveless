file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
add_library(WsRuntime ${HEADERS} ${SOURCES})
set_property(TARGET WsRuntime PROPERTY POSITION_INDEPENDENT_CODE ON)