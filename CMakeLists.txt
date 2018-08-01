cmake_minimum_required(VERSION 3.1)

project(lightstep-tracer)

# ==============================================================================
# Version information

set(LIGHTSTEP_VERSION_MAJOR "0")
set(LIGHTSTEP_VERSION_MINOR "8")
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

option(WITH_GRPC "Build with support for gRPC." ON)
option(WITH_DYNAMIC_LOAD "Build support for dynamic loading." ON)
option(ENABLE_LINTING "Run clang-tidy on sources if available." OFF)
option(HEADERS_ONLY "Only generate config.h and version.h." OFF)

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
elseif(WITH_DYNAMIC_LOAD)
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
configure_file(config.h.in include/lightstep/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lightstep 
        DESTINATION include)

if(HEADERS_ONLY)
  return()
endif()

# ==============================================================================
# Configure compiler warnings

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(WARNING_CXX_FLAGS -Weverything 
    -Wno-c++98-compat 
    -Wno-c++98-compat-bind-to-temporary-copy 
    -Wno-weak-vtables 
    -Wno-exit-time-destructors 
    -Wno-global-constructors 
    -Wno-sign-conversion
    -Wno-padded
    -Wno-switch-enum
    -Wno-disabled-macro-expansion)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(WARNING_CXX_FLAGS -Wall -Wextra)
endif()

# ==============================================================================
# Find packages

find_package(Protobuf REQUIRED)

if (NOT DEFINED OPENTRACING_INCLUDE_DIR)
  find_path(OPENTRACING_INCLUDE_DIR NAMES opentracing/tracer.h)
endif()

if (NOT DEFINED OPENTRACING_LIBRARY)
  find_library(OPENTRACING_LIBRARY opentracing)
endif()

include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})

set(LIGHTSTEP_LINK_LIBRARIES ${OPENTRACING_LIBRARY}
                             ${PROTOBUF_LIBRARIES})

if (WITH_GRPC)                           
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

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
list(APPEND LIGHTSTEP_LINK_LIBRARIES Threads::Threads)

# ==============================================================================
# Build LightStep tracer library

add_subdirectory(3rd_party)
include_directories(SYSTEM ${LIGHTSTEP_THIRD_PARTY_INCLUDES})

include_directories(include)
install(DIRECTORY include/lightstep DESTINATION include)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(LightStepTracerCommon)
include(LightStepTracerConfiguration)

set(LIGHTSTEP_SRCS src/utility.cpp
                   src/in_memory_stream.cpp
                   src/logger.cpp
                   src/propagation.cpp
                   src/binary_carrier.cpp
                   src/grpc_transporter.cpp
                   src/report_builder.cpp
                   src/manual_recorder.cpp
                   src/auto_recorder.cpp
                   src/lightstep_span_context.cpp
                   src/lightstep_span.cpp
                   src/lightstep_tracer_impl.cpp
                   src/lightstep_tracer_factory.cpp
                   src/transporter.cpp
                   src/tracer.cpp
                   )

if (WITH_DYNAMIC_LOAD)
  list(APPEND LIGHTSTEP_SRCS src/dynamic_load.cpp)
endif()

if (DEFAULT_SSL_ROOTS_PEM STREQUAL "")
  list(APPEND LIGHTSTEP_SRCS src/no_default_ssl_roots_pem.cpp)
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
  if (ENABLE_LINTING)
    include(LightStepClangTidy)
    _apply_clang_tidy_if_available(lightstep_tracer)
  endif()
  target_link_libraries(lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
  install(TARGETS lightstep_tracer
          LIBRARY DESTINATION lib)
endif()

if (BUILD_STATIC_LIBS)  
  add_library(lightstep_tracer-static STATIC $<TARGET_OBJECTS:lightstep_tracer_common>
                                             $<TARGET_OBJECTS:lightstep_tracer_configuration>
                                             $<TARGET_OBJECTS:lightstep_3rd_party>
                                             ${LIGHTSTEP_SRCS})
  set_target_properties(lightstep_tracer-static PROPERTIES OUTPUT_NAME lightstep_tracer)
  target_compile_options(lightstep_tracer-static PUBLIC ${WARNING_CXX_FLAGS})
  if (ENABLE_LINTING)
    include(LightStepClangTidy)
    _apply_clang_tidy_if_available(lightstep_tracer-static)
  endif()
  target_link_libraries(lightstep_tracer-static ${LIGHTSTEP_LINK_LIBRARIES})
  install(TARGETS lightstep_tracer-static
          ARCHIVE DESTINATION lib)
endif()


# ==============================================================================
# Build tests and examples

include(CTest)
if (BUILD_TESTING AND BUILD_SHARED_LIBS)
  add_subdirectory(test)
  add_subdirectory(example)
endif()
