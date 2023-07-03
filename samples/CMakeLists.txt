cmake_minimum_required(VERSION 3.22.1)

project(samples)

# Allow to use `$ make VERBOSE=1;`
set(CMAKE_VERBOSE_MAKEFILE, on)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

# ----------------------------------------------------------

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shared/imgui_application)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sample_ImGui_Demo/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sample_ImPlot_Demo/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sample_ImSpinner_Demo/)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sample_Yoga_Playground/)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sample_FlatBuffers_Compiler/)
