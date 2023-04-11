cmake_minimum_required(VERSION 3.22.1)

project(Yoga)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

message(STATUS "[Yoga] Build type: ${CMAKE_BUILD_TYPE}")

# ----------------------------------------------------------

set(YOGA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/yoga-main/")

set(LIB_INCLUDE_DIRECTORIES
    ${YOGA_DIR}/
    ${YOGA_DIR}/yoga/
)

set(YOGA_INCLUDE_DIRECTORIES
    ${YOGA_DIR}/
    ${YOGA_DIR}/yoga/

    PARENT_SCOPE
)

set(LIB_HEADERS
    ${YOGA_DIR}/yoga/event/event.h
    ${YOGA_DIR}/yoga/BitUtils.h
    ${YOGA_DIR}/yoga/CompactValue.h
    ${YOGA_DIR}/yoga/log.h
    ${YOGA_DIR}/yoga/Utils.h
    ${YOGA_DIR}/yoga/YGConfig.h
    ${YOGA_DIR}/yoga/YGEnums.h
    ${YOGA_DIR}/yoga/YGFloatOptional.h
    ${YOGA_DIR}/yoga/YGLayout.h
    ${YOGA_DIR}/yoga/YGMacros.h
    ${YOGA_DIR}/yoga/YGNode.h
    ${YOGA_DIR}/yoga/YGNodePrint.h
    ${YOGA_DIR}/yoga/YGStyle.h
    ${YOGA_DIR}/yoga/YGValue.h
    ${YOGA_DIR}/yoga/Yoga-internal.h
    ${YOGA_DIR}/yoga/Yoga.h
)

set(LIB_SOURCES
    ${YOGA_DIR}/yoga/event/event.cpp
    ${YOGA_DIR}/yoga/log.cpp
    ${YOGA_DIR}/yoga/Utils.cpp
    ${YOGA_DIR}/yoga/YGConfig.cpp
    ${YOGA_DIR}/yoga/YGEnums.cpp
    ${YOGA_DIR}/yoga/YGLayout.cpp
    ${YOGA_DIR}/yoga/YGNode.cpp
    ${YOGA_DIR}/yoga/YGNodePrint.cpp
    ${YOGA_DIR}/yoga/YGStyle.cpp
    ${YOGA_DIR}/yoga/YGValue.cpp
    ${YOGA_DIR}/yoga/Yoga.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${LIB_HEADERS} ${LIB_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIB_INCLUDE_DIRECTORIES}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wformat -Wextra -Wpedantic
)

if(EMSCRIPTEN)

    message(STATUS "[Yoga] Platform: EMSCRIPTEN")

else()

    message(STATUS "[Yoga] Platform: Native")

endif()
