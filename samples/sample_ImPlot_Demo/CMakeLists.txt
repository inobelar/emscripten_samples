cmake_minimum_required(VERSION 3.22.1)

project(sample_ImPlot_Demo)

# ------------------------------------------------------------------------------

# Allow to use `$ make VERBOSE=1;`
set(CMAKE_VERBOSE_MAKEFILE, on)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

# ------------------------------------------------------------------------------

# Guard for standalone build
if(PROJECT_IS_TOP_LEVEL)

    # Add 'ImGui Application' library
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/imgui_application

        # Special name needed due to CMake beaviour, when sub-directory located
        # not in sub-directory, but in outer space
        "imgui_application_build_for_${PROJECT_NAME}"
    )

endif()

# ------------------------------------------------------------------------------
# ImPlot library

set(IMPLOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared/implot/implot-0.14/")

set(IMPLOT_INCLUDE_DIRECTORIES
    ${IMPLOT_DIR}
)

set(IMPLOT_HEADERS
    ${IMPLOT_DIR}/implot.h
    ${IMPLOT_DIR}/implot_internal.h
)

set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp

    ${IMPLOT_DIR}/implot_demo.cpp
)

# ------------------------------------------------------------------------------

add_executable(${PROJECT_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"

    ${IMPLOT_HEADERS} ${IMPLOT_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    IMGUI_DEFINE_MATH_OPERATORS
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${IMGUI_APPLICATION_INCLUDE_DIRECTORIES}

    ${IMPLOT_INCLUDE_DIRECTORIES}
)

# ------------------------------------------------------------------------------

if(EMSCRIPTEN)

    # IMPORTANT NOTE: due to `target_link_options` deduplication,
    # we use "-sWASM=1 -sUSE_SDL=2 ..." which works well, instead of
    # "-s WASM=1 -s USE_SDL=2 ..." (with spaces), which becomes
    # "-s WASM=1 USE_SDL=2 ..." (duplicated but necessary '-s' removed).

    target_link_options(${PROJECT_NAME} PRIVATE
        -sWASM=1
        -sUSE_SDL=2

        -sUSE_WEBGL2=1
        #-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 # Only target for WebGL2 (drop support for WebGL1 to save code size)

        -sALLOW_MEMORY_GROWTH=1
        -sDISABLE_EXCEPTION_CATCHING=1 -sNO_EXIT_RUNTIME=0 -sASSERTIONS=1

        -sNO_FILESYSTEM=1

        -sSINGLE_FILE

        --shell-file ${IMGUI_APPLICATION_SHELL_MINIMAL}
    )

    target_link_libraries(${PROJECT_NAME}
        imgui_application
    )

    set(CMAKE_EXECUTABLE_SUFFIX ".html")

else()

    target_link_libraries(${PROJECT_NAME}
        imgui_application
    )

endif()

# ------------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME})
