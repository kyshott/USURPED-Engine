cmake_minimum_required(VERSION 3.24)

project(A4Student)

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


#
# Dependencies
#


# Projects
#
add_subdirectory(src)
add_subdirectory(external/imgui)
add_subdirectory(external/raylib)




