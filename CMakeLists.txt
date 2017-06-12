cmake_minimum_required(VERSION 3.0)
enable_testing()

project(lightstep-tracer)

find_package(Protobuf REQUIRED)

find_path(OPENTRACING_INCLUDE_DIR NAMES opentracing/tracer.h)
find_library(OPENTRACING_LIB opentracing)

set(PROTO_PATH "${CMAKE_SOURCE_DIR}/lightstep-tracer-common")

if( NOT EXISTS "${PROTO_PATH}/.git" )
  message(SEND_ERROR "The git submodules are not available. \
Please run git submodule update --init --recursive")
endif()

include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})

add_subdirectory(c++11)
