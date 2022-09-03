cmake_minimum_required (VERSION 3.8)

project(AsepriteRenderHook)

add_executable (
	AsepriteRenderHook
	AsepriteRenderHook.cpp
	AsepriteRenderHook.h
	Websocket.h
	Websocket.cpp
	main.cpp "DemoWindow.h" "DemoWindow.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AsepriteRenderHook PROPERTY CXX_STANDARD 20)
endif()

find_package(websocketpp CONFIG REQUIRED)

find_package(boost 1.79.0 REQUIRED system)
find_package(boost 1.79.0 REQUIRED COMPONENTS asio)

target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		Engine
	PRIVATE
		websocketpp::websocketpp
)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/../Engine/shaders ${PROJECT_BINARY_DIR}/shaders
)