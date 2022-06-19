cmake_minimum_required(VERSION 3.8)

project(Engine)

# vulkan
if (DEFINED VULKAN_SDK_PATH)
	set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
  set(Vulkan_FOUND "True")
else()
	find_package(Vulkan REQUIRED)
	message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find the Vulkan library!")
else()
	message(STATUS "Using vulkan lib at ${Vulkan_LIBRARIES}")
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

# imgui
find_package(imgui CONFIG REQUIRED) 
if(NOT imgui_FOUND)
  message(STATUS "could not find imgui using vcpkg")
endif()

add_library(${PROJECT_NAME}
  Renderer.cpp
  Pipeline.h
  Pipeline.cpp
  Device.h
  Device.cpp
  Window.h
  Window.cpp
  Swapchain.h
  Swapchain.cpp
  Model.h
  Model.cpp
  Entity.h
  Entity.cpp
  Engine.h 
  Engine.cpp
  RenderSystem.h
  RenderSystem.cpp
  UserInterface.h
  UserInterface.cpp
  Buffer.h
  Buffer.cpp
)

target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}
		${Vulkan_INCLUDE_DIRS}
   	${GLFW_INCLUDE_DIRS}
    ${GLM_PATH}
)

target_link_directories(${PROJECT_NAME}
	PUBLIC
		${Vulkan_LIBRARIES}
    ${GLFW_LIB}
)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw3 imgui::imgui)

## build shader files to spir-v
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)

# get all GLSL files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
  "${PROJECT_SOURCE_DIR}/shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_custom_target(copyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/shaders ${PROJECT_BINARY_DIR}/shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(Engine Shaders copyShaders)