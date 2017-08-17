cmake_minimum_required(VERSION 3.0)
enable_testing()

project(lightstep-tracer)

option(LS_WITH_ASAN "Generate tests using address-sanitizer." OFF)
option(LS_WITH_TSAN "Generate tests using thread-sanitizer." OFF)
option(LS_WITH_GRPC "Build with support for gRPC." ON)

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

set(LIGHTSTEP_LINK_LIBRARIES ${OPENTRACING_LIB}
                             ${PROTOBUF_LIBRARIES} 
                             ${GRPC_LIBRARIES}
                             ${GRPCPP_LIBRARIES})

set(WITH_ASAN OFF)
if (LS_WITH_ASAN AND (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
  ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND
   "${COMPILER_VERSION}" VERSION_GREATER "4.8")))
   set(WITH_ASAN ON)
   set(ASAN_CXX_FLAGS -O1 -g -fsanitize=address -fno-omit-frame-pointer)
   set(ASAN_LD_FLAGS -fsanitize=address)
endif()

set(WITH_TSAN OFF)
if (LS_WITH_TSAN AND (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
  ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND
   "${COMPILER_VERSION}" VERSION_GREATER "4.8")))
   set(WITH_TSAN ON)
   set(TSAN_CXX_FLAGS -O1 -g -fsanitize=thread)
   set(TSAN_LD_FLAGS -fsanitize=thread)
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${GRPC_CFLAGS_OTHER} \
                     ${GRPCPP_CFLAGS_OTHER}")

include_directories(SYSTEM ${GRPC_INCLUDE_DIRS} ${GRPCPP_INCLUDE_DIRS})
include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})

add_subdirectory(c++11)
