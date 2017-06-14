cmake_minimum_required(VERSION 3.0)
enable_testing()

project(lightstep-tracer)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
                  "A LightStep implementation of the C++ OpenTracing API")
SET(CPACK_PACKAGE_VENDOR "lightstep.com")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "4")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
include(cpack)

find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)

pkg_search_module(GRPC REQUIRED grpc)
pkg_search_module(GRPCPP REQUIRED grpc++)

find_path(OPENTRACING_INCLUDE_DIR NAMES opentracing/tracer.h)
find_library(OPENTRACING_LIB opentracing)

set(PROTO_PATH "${CMAKE_SOURCE_DIR}/lightstep-tracer-common")

if( NOT EXISTS "${PROTO_PATH}/.git" )
  execute_process(COMMAND git submodule update --init --recursive
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})

add_subdirectory(c++11)
