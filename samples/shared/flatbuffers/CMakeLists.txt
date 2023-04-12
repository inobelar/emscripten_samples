cmake_minimum_required(VERSION 3.22.1)

project(FlatBuffers)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

message(STATUS "[FlatBuffers] Build type: ${CMAKE_BUILD_TYPE}")

# ----------------------------------------------------------

set(FLATBUFFERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers-23.3.3/")

# gRPC - dependencies, needed to compile 'src/idl_gen_grpc.cpp'

set(LIB_INCLUDE_DIRECTORIES
    ${FLATBUFFERS_DIR}/include/
    ${FLATBUFFERS_DIR}/src/
    
    # gRPC
    ${FLATBUFFERS_DIR}/grpc/
)

set(FLATBUFFERS_INCLUDE_DIRECTORIES
    ${FLATBUFFERS_DIR}/include
    ${FLATBUFFERS_DIR}/src/
    
    # gRPC
    ${FLATBUFFERS_DIR}/grpc/

    PARENT_SCOPE
)

set(LIB_HEADERS
    ${FLATBUFFERS_DIR}/src/annotated_binary_text_gen.h
    ${FLATBUFFERS_DIR}/src/bfbs_gen.h
    ${FLATBUFFERS_DIR}/src/bfbs_gen_lua.h
    ${FLATBUFFERS_DIR}/src/bfbs_gen_nim.h
    ${FLATBUFFERS_DIR}/src/bfbs_namer.h
    ${FLATBUFFERS_DIR}/src/binary_annotator.h
    ${FLATBUFFERS_DIR}/src/idl_gen_binary.h
    ${FLATBUFFERS_DIR}/src/idl_gen_cpp.h
    ${FLATBUFFERS_DIR}/src/idl_gen_csharp.h
    ${FLATBUFFERS_DIR}/src/idl_gen_dart.h
    ${FLATBUFFERS_DIR}/src/idl_gen_fbs.h
    ${FLATBUFFERS_DIR}/src/idl_gen_go.h
    ${FLATBUFFERS_DIR}/src/idl_gen_java.h
    ${FLATBUFFERS_DIR}/src/idl_gen_json_schema.h
    ${FLATBUFFERS_DIR}/src/idl_gen_kotlin.h
    ${FLATBUFFERS_DIR}/src/idl_gen_lobster.h
    ${FLATBUFFERS_DIR}/src/idl_gen_lua.h
    ${FLATBUFFERS_DIR}/src/idl_gen_php.h
    ${FLATBUFFERS_DIR}/src/idl_gen_python.h
    ${FLATBUFFERS_DIR}/src/idl_gen_rust.h
    ${FLATBUFFERS_DIR}/src/idl_gen_swift.h
    ${FLATBUFFERS_DIR}/src/idl_gen_text.h
    ${FLATBUFFERS_DIR}/src/idl_gen_ts.h
    ${FLATBUFFERS_DIR}/src/idl_namer.h
    ${FLATBUFFERS_DIR}/src/namer.h
    
    # gRPC
    ${FLATBUFFERS_DIR}/grpc/src/compiler/cpp_generator.h
    ${FLATBUFFERS_DIR}/grpc/src/compiler/go_generator.h
    ${FLATBUFFERS_DIR}/grpc/src/compiler/java_generator.h
    ${FLATBUFFERS_DIR}/grpc/src/compiler/python_generator.h
    ${FLATBUFFERS_DIR}/grpc/src/compiler/schema_interface.h
    ${FLATBUFFERS_DIR}/grpc/src/compiler/swift_generator.h
    ${FLATBUFFERS_DIR}/grpc/src/compiler/ts_generator.h
)

set(LIB_SOURCES
    ${FLATBUFFERS_DIR}/src/annotated_binary_text_gen.cpp
    ${FLATBUFFERS_DIR}/src/bfbs_gen_lua.cpp
    ${FLATBUFFERS_DIR}/src/bfbs_gen_nim.cpp
    ${FLATBUFFERS_DIR}/src/binary_annotator.cpp
    ${FLATBUFFERS_DIR}/src/code_generators.cpp
    ${FLATBUFFERS_DIR}/src/flatc.cpp
#   ${FLATBUFFERS_DIR}/src/flatc_main.cpp        # <-- This is compiler entry point, but we need library instead of executable, so it's skipped
    ${FLATBUFFERS_DIR}/src/flathash.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_binary.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_cpp.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_csharp.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_dart.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_fbs.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_go.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_grpc.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_java.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_json_schema.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_kotlin.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_lobster.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_lua.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_php.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_python.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_rust.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_swift.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_text.cpp
    ${FLATBUFFERS_DIR}/src/idl_gen_ts.cpp
    ${FLATBUFFERS_DIR}/src/idl_parser.cpp
    ${FLATBUFFERS_DIR}/src/reflection.cpp
    ${FLATBUFFERS_DIR}/src/util.cpp
    
    # gRPC
    ${FLATBUFFERS_DIR}/grpc/src/compiler/cpp_generator.cc
    ${FLATBUFFERS_DIR}/grpc/src/compiler/go_generator.cc
    ${FLATBUFFERS_DIR}/grpc/src/compiler/java_generator.cc
    ${FLATBUFFERS_DIR}/grpc/src/compiler/python_generator.cc
    ${FLATBUFFERS_DIR}/grpc/src/compiler/swift_generator.cc
    ${FLATBUFFERS_DIR}/grpc/src/compiler/ts_generator.cc
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

    message(STATUS "[FlatBuffers] Platform: EMSCRIPTEN")

else()

    message(STATUS "[FlatBuffers] Platform: Native")

endif()
