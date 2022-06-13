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

# imgui
#file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
#add_library("ImGui" STATIC ${IMGUI_SOURCES})

add_library(${PROJECT_NAME} Renderer.cpp "Pipeline.h" "Pipeline.cpp" "Device.h" "Device.cpp"  )

target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}
		${Vulkan_INCLUDE_DIRS}
)

target_link_directories(${PROJECT_NAME}
	PUBLIC
		${Vulkan_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} vulkan-1)

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

add_dependencies(Engine Shaders)