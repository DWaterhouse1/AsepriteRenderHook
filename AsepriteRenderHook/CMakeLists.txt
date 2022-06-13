cmake_minimum_required (VERSION 3.8)

project(AsepriteRenderHook)

add_executable (AsepriteRenderHook "AsepriteRenderHook.cpp" "AsepriteRenderHook.h" "main.cpp"   "Window.h" "Window.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AsepriteRenderHook PROPERTY CXX_STANDARD 20)
endif()

# glfw
if (DEFINED GLFW_PATH)
	message(STATUS "using GLFW path specified in .env")
	set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
	if(MSVC)
		set(GLFW_LIB "${GLFW_PATH}/lib-vc2022") # check that this is the correct version of visual studio
	elseif(CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
		message(STATUS "USING MINGW")
    set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
	endif()
else()
	find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
  message(STATUS "Found GLFW")
endif()
if (NOT GLFW_LIB)
	message(FATAL_ERROR "Could not find glfw library!")
else()
	message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}
		${GLFW_INCLUDE_DIRS}
)

target_link_directories(${PROJECT_NAME}
	PUBLIC
    ${GLFW_LIB}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		Engine
		glfw3
)