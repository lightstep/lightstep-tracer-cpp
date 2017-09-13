cmake_minimum_required(VERSION 3.0)
enable_testing()

project(lightstep-tracer)

# ==============================================================================
# Version information

set(LIGHTSTEP_VERSION_MAJOR "0")
set(LIGHTSTEP_VERSION_MINOR "4")
set(LIGHTSTEP_VERSION_PATCH "0")
set(LIGHTSTEP_VERSION_STRING
  "${LIGHTSTEP_VERSION_MAJOR}.${LIGHTSTEP_VERSION_MINOR}.${LIGHTSTEP_VERSION_PATCH}")

# ==============================================================================
# Set up cpack

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
                  "A LightStep implementation of the C++ OpenTracing API")
set(CPACK_PACKAGE_VENDOR "lightstep.com")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_VERSION_MAJOR ${LIGHTSTEP_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIGHTSTEP_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIGHTSTEP_VERSION_PATCH})
include(CPack)


# ==============================================================================
# Set up options

option(LS_WITH_ASAN "Generate tests using address-sanitizer." OFF)
option(LS_WITH_TSAN "Generate tests using thread-sanitizer." OFF)
option(LS_WITH_GRPC "Build with support for gRPC." ON)


# ==============================================================================
# Find packages

find_package(Protobuf REQUIRED)

find_path(OPENTRACING_INCLUDE_DIR NAMES opentracing/tracer.h)
find_library(OPENTRACING_LIB opentracing)

include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})

set(LIGHTSTEP_LINK_LIBRARIES ${OPENTRACING_LIB}
                             ${PROTOBUF_LIBRARIES})

if (LS_WITH_GRPC)                           
  find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
  if (NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "grpc_cpp_plugin not found!")
  endif()
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GRPC REQUIRED grpc)
  pkg_search_module(GRPCPP REQUIRED grpc++)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${GRPC_CFLAGS} \
                       ${GRPCPP_CFLAGS}")
  list(APPEND LIGHTSTEP_LINK_LIBRARIES ${GRPC_LDFLAGS} ${GRPCPP_LDFLAGS})  
  include_directories(SYSTEM ${GRPC_INCLUDE_DIRS} ${GRPCPP_INCLUDE_DIRS})
endif()

# ==============================================================================
# Configure sanitizers

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

add_subdirectory(c++11)
