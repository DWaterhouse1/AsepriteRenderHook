include(Windows.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project (AsepriteRenderHook VERSION 0.0.1)

# Include sub-projects.
add_subdirectory(AsepriteRenderHook)
add_subdirectory(Engine)