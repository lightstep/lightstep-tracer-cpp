cmake_minimum_required(VERSION 3.1)

# ==============================================================================
# Set policies

# Allows us to control options via normal variables
# this is used for the windows build
# See https://cmake.org/cmake/help/git-stage/policy/CMP0077.html
if(NOT (${CMAKE_VERSION} VERSION_LESS "3.13.0"))
  cmake_policy(SET CMP0077 NEW)
endif()

# Allows us to specify the MSVC runtime via cmake variables
# See https://cmake.org/cmake/help/git-stage/policy/CMP0091.html
if(NOT (${CMAKE_VERSION} VERSION_LESS "3.15.0"))
  cmake_policy(SET CMP0091 NEW)
endif()

project(lightstep-tracer)

# ==============================================================================
# Version information

set(LIGHTSTEP_VERSION_MAJOR "0")
set(LIGHTSTEP_VERSION_MINOR "13")
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

# Introduce variables:
# * CMAKE_INSTALL_LIBDIR
# * CMAKE_INSTALL_BINDIR
# * CMAKE_INSTALL_INCLUDEDIR
include(GNUInstallDirs)

option(WITH_GRPC "Build with support for gRPC." ON)
option(WITH_LIBEVENT "Build with support for libevent." OFF)
option(WITH_CARES "Build with support for dns resolution using c-ares." OFF)
option(WITH_DYNAMIC_LOAD "Build support for dynamic loading." ON)
option(HEADERS_ONLY "Only generate version.h." OFF)

# Allow a user to specify an optional default roots.pem file to embed into the 
# library. 
#
# This is useful if the library is distributed to an environment where gRPC
# hasn't been installed.
#
# To use, invoke cmake with 
#   cmake -DDEFAULT_SSL_ROOTS_PEM:STRING=/path/to/roots.pem ...
#
# See also discussion on https://github.com/grpc/grpc/issues/4834
set(DEFAULT_SSL_ROOTS_PEM "" CACHE STRING "Path to a default roots.pem file to embed")

if (WITH_GRPC)
  set(LIGHTSTEP_USE_GRPC 1)
elseif(WITH_DYNAMIC_LOAD AND NOT WITH_LIBEVENT)
  message(WARNING "WITH_GRPC is not set; building without dynamic loading support.")
  set(WITH_DYNAMIC_LOAD 0)
endif()

option(BUILD_SHARED_LIBS "Build as a shared library" ON)
option(BUILD_STATIC_LIBS "Build as a static library" ON)

if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(FATAL_ERROR "One or both of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be set to ON to build")
endif()

# ==============================================================================
# Set up generated header files config.h and version.h

configure_file(version.h.in include/lightstep/version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lightstep 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(HEADERS_ONLY)
  return()
endif()

# ==============================================================================
# Configure compiler warnings

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Find packages

set(LIGHTSTEP_LINK_LIBRARIES "")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Newer protobuf versions use cmake config files so try that first
#
# Note: The non-config protobuf module doesn't find PROTOBUF_PROTOC_EXECUTABLE properly on windows.
set(protobuf_MODULE_COMPATIBLE ON)
find_package(Protobuf CONFIG NAMES protobuf)
if (protobuf_FOUND)
  list(APPEND LIGHTSTEP_LINK_LIBRARIES protobuf::libprotobuf)
else()
  # Fall back to the old style protobuf module
  find_package(Protobuf REQUIRED)
  list(APPEND LIGHTSTEP_LINK_LIBRARIES ${PROTOBUF_LIBRARIES})
endif()

if (NOT DEFINED OPENTRACING_INCLUDE_DIR)
  find_path(OPENTRACING_INCLUDE_DIR NAMES opentracing/tracer.h)
endif()

if (NOT DEFINED OPENTRACING_LIBRARY)
  find_library(OPENTRACING_LIBRARY opentracing)
endif()

include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
list(APPEND LIGHTSTEP_LINK_LIBRARIES ${OPENTRACING_LIBRARY})

if (WITH_GRPC)                           
  find_package(gRPC CONFIG)
  # First attempt to set up gRPC via cmake; but if cmake config files aren't
  # available, fallback to pkg-config.
  if (gRPC_FOUND)
    set(GRPC_CPP_PLUGIN $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
    list(APPEND LIGHTSTEP_LINK_LIBRARIES gRPC::grpc++)
    include_directories(SYSTEM
          $<TARGET_PROPERTY:gRPC::grpc++,INTERFACE_INCLUDE_DIRECTORIES>)
  else()
    message("Falling back to finding gRPC with pkg-config")
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    if (NOT GRPC_CPP_PLUGIN)
      message(FATAL_ERROR "grpc_cpp_plugin not found!")
    endif()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GRPC REQUIRED grpc)
    pkg_search_module(GRPCPP REQUIRED grpc++)
    list(APPEND LIGHTSTEP_LINK_LIBRARIES ${GRPCPP_LDFLAGS} ${GRPC_LDFLAGS})  
    include_directories(SYSTEM ${GRPC_INCLUDE_DIRS} ${GRPCPP_INCLUDE_DIRS})
  endif()
endif()

if (WITH_LIBEVENT)
  find_package(Libevent REQUIRED)
  list(APPEND LIGHTSTEP_LINK_LIBRARIES   ${LIBEVENT_LIBRARIES})
  include_directories(SYSTEM ${LIBEVENT_INCLUDE_DIRS})
endif()

if (WITH_CARES)
  find_package(CARES REQUIRED)
  list(APPEND LIGHTSTEP_LINK_LIBRARIES   ${CARES_LIBRARIES})
  include_directories(SYSTEM ${CARES_INCLUDE_DIR})
endif()

if (WITH_LIBEVENT AND WITH_CARES)
  set(LIGHTSTEP_USE_STREAMING 1)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
list(APPEND LIGHTSTEP_LINK_LIBRARIES Threads::Threads)

if (WIN32) 
  list(APPEND LIGHTSTEP_LINK_LIBRARIES "wsock32" "ws2_32")
endif()


# ==============================================================================
# Build LightStep tracer library

add_library(dummy_version_check
          src/common/version_check.cpp)
target_link_libraries(dummy_version_check ${OPENTRACING_LIBRARY})

add_subdirectory(3rd_party)
include_directories(SYSTEM ${LIGHTSTEP_THIRD_PARTY_INCLUDES})

include_directories(include)
include_directories(src)
install(DIRECTORY include/lightstep DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(LightStepTracerCommon)
include(LightStepTracerConfiguration)

set(LIGHTSTEP_SRCS src/common/utility.cpp
                   src/common/buffer_chain.cpp
                   src/common/fragment_input_stream.cpp
                   src/common/fragment_array_input_stream.cpp
                   src/common/protobuf.cpp
                   src/common/hex_conversion.cpp
                   src/common/in_memory_stream.cpp
                   src/common/logger.cpp
                   src/common/random.cpp
                   src/common/random_traverser.cpp
                   src/common/serialization.cpp
                   src/common/chunked_http_framing.cpp
                   src/common/report_request_framing.cpp
                   src/common/composable_fragment_input_stream.cpp
                   src/common/chained_stream.cpp
                   src/common/timestamp.cpp
                   src/recorder/report_builder.cpp
                   src/recorder/auto_recorder.cpp
                   src/recorder/fork_aware_recorder.cpp
                   src/recorder/legacy_manual_recorder.cpp
                   src/recorder/manual_recorder.cpp
                   src/recorder/metrics_tracker.cpp
                   src/recorder/transporter.cpp
                   src/recorder/serialization/report_request.cpp
                   src/recorder/serialization/report_request_header.cpp
                   src/recorder/serialization/embedded_metrics_message.cpp
                   src/tracer/binary_carrier.cpp
                   src/tracer/json_options.cpp
                   src/tracer/propagation/b3_propagator.cpp
                   src/tracer/propagation/baggage_propagator.cpp
                   src/tracer/propagation/binary_propagation.cpp
                   src/tracer/propagation/envoy_propagator.cpp
                   src/tracer/propagation/trace_context.cpp
                   src/tracer/propagation/trace_context_propagator.cpp
                   src/tracer/propagation/lightstep_propagator.cpp
                   src/tracer/propagation/multiheader_propagator.cpp
                   src/tracer/propagation/cloud_trace_propagator.cpp
                   src/tracer/propagation/propagation.cpp
                   src/tracer/propagation/propagation_options.cpp
                   src/tracer/immutable_span_context.cpp
                   src/tracer/legacy/legacy_span.cpp
                   src/tracer/legacy/legacy_tracer_impl.cpp
                   src/tracer/lightstep_span_context.cpp
                   src/tracer/lightstep_tracer_factory.cpp
                   src/tracer/serialization.cpp
                   src/tracer/span.cpp
                   src/tracer/tracer_impl.cpp
                   src/tracer/tracer.cpp
                   src/tracer/tag.cpp
                   src/tracer/utility.cpp
                   )

if (WIN32)
  list(APPEND LIGHTSTEP_SRCS src/common/platform/error_windows.cpp
                             src/common/platform/network_windows.cpp
                             src/common/platform/network_environment_windows.cpp
                             src/common/platform/utility_windows.cpp
                             src/common/platform/string_windows.cpp
                             src/common/platform/fork_windows.cpp)
else()
  list(APPEND LIGHTSTEP_SRCS src/common/platform/error_unix.cpp
                             src/common/platform/network_unix.cpp
                             src/common/platform/network_environment_unix.cpp
                             src/common/platform/utility_unix.cpp
                             src/common/platform/string_unix.cpp
                             src/common/platform/fork_unix.cpp)
endif()

if (WITH_GRPC)
  list(APPEND LIGHTSTEP_SRCS src/recorder/grpc_transporter/grpc_transporter.cpp)
else()
  list(APPEND LIGHTSTEP_SRCS src/recorder/no_grpc_transporter.cpp)
endif()

if (WITH_LIBEVENT)
  list(APPEND LIGHTSTEP_SRCS src/recorder/stream_recorder/stream_recorder.cpp
                             src/recorder/stream_recorder/stream_recorder_impl.cpp
                             src/recorder/stream_recorder/satellite_dns_resolution_manager.cpp
                             src/recorder/stream_recorder/satellite_endpoint_manager.cpp
                             src/recorder/stream_recorder/satellite_connection.cpp
                             src/recorder/stream_recorder/satellite_streamer.cpp
                             src/recorder/stream_recorder/span_stream.cpp
                             src/recorder/metrics_tracker.cpp
                             src/recorder/stream_recorder/connection_stream.cpp
                             src/recorder/stream_recorder/host_header.cpp
                             src/recorder/stream_recorder/status_line_parser.cpp
                             src/recorder/stream_recorder/utility.cpp
                             src/network/event.cpp
                             src/network/event_base.cpp
                             src/network/timer_event.cpp
                             src/network/ip_address.cpp
                             src/network/socket.cpp
                             src/network/vector_write.cpp
  )
  if (WITH_CARES)
    list(APPEND LIGHTSTEP_SRCS src/network/ares_dns_resolver/ares_dns_resolver.cpp
                               src/network/ares_dns_resolver/ares_library_handle.cpp
    )
  else()
    list(APPEND LIGHTSTEP_SRCS src/network/no_dns_resolver.cpp)
  endif()
else()
  list(APPEND LIGHTSTEP_SRCS src/recorder/no_stream_recorder.cpp)
endif()

if (WITH_DYNAMIC_LOAD)
  list(APPEND LIGHTSTEP_SRCS src/tracer/dynamic_load.cpp)
endif()

if (DEFAULT_SSL_ROOTS_PEM STREQUAL "")
  list(APPEND LIGHTSTEP_SRCS src/tracer/no_default_ssl_roots_pem.cpp)
else()
  # Follows the approach described in https://stackoverflow.com/a/11814544/4447365
  set(EMBED_SSL_ROOTS_PEM_CPP_FILE ${CMAKE_BINARY_DIR}/default_ssl_roots_pem.cpp)
  add_custom_command(
    OUTPUT ${EMBED_SSL_ROOTS_PEM_CPP_FILE}
    COMMAND embedfile default_ssl_roots_pem ${DEFAULT_SSL_ROOTS_PEM}
    DEPENDS ${DEFAULT_ROOT_PEM}
  )
  list(APPEND LIGHTSTEP_SRCS ${EMBED_SSL_ROOTS_PEM_CPP_FILE})
endif()

if (BUILD_SHARED_LIBS)  
  add_library(lightstep_tracer SHARED $<TARGET_OBJECTS:lightstep_tracer_common>
                                      $<TARGET_OBJECTS:lightstep_tracer_configuration>
                                      $<TARGET_OBJECTS:lightstep_3rd_party>
                                      ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer PUBLIC ${WARNING_CXX_FLAGS})
  target_link_libraries(lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
  set_target_properties(lightstep_tracer PROPERTIES SOVERSION ${LIGHTSTEP_VERSION_MAJOR})
  install(TARGETS lightstep_tracer
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if (BUILD_STATIC_LIBS)  
  add_library(lightstep_tracer-static STATIC $<TARGET_OBJECTS:lightstep_tracer_common>
                                             $<TARGET_OBJECTS:lightstep_tracer_configuration>
                                             $<TARGET_OBJECTS:lightstep_3rd_party>
                                             ${LIGHTSTEP_SRCS})
  set_target_properties(lightstep_tracer-static PROPERTIES OUTPUT_NAME lightstep_tracer)
  target_compile_options(lightstep_tracer-static PUBLIC ${WARNING_CXX_FLAGS})
  target_link_libraries(lightstep_tracer-static ${LIGHTSTEP_LINK_LIBRARIES})
  install(TARGETS lightstep_tracer-static
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# ==============================================================================
# Build tests and examples
if (BUILD_SHARED_LIBS)
  set(LIGHTSTEP_LIBRARY "lightstep_tracer")
else()
  set(LIGHTSTEP_LIBRARY "lightstep_tracer-static")
endif()

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test/cmake)
  add_subdirectory(example)
endif()
