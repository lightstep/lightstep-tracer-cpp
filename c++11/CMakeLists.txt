set(CMAKE_CXX_STANDARD 11)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(WARNING_CXX_FLAGS -Weverything 
    -Wno-c++98-compat 
    -Wno-c++98-compat-bind-to-temporary-copy 
    -Wno-weak-vtables 
    -Wno-exit-time-destructors 
    -Wno-global-constructors 
    -Wno-sign-conversion
    -Wno-padded)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(WARNING_CXX_FLAGS -Wall -Wextra)
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy" 
                            DOC "Path to clang-tidy executable")
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*\
,-llvm-include-order,-google-runtime-references,-google-build-using-namespace")
endif()

configure_file(version.h.in include/lightstep/version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lightstep 
        DESTINATION include)

include_directories(SYSTEM 3rd_party/include)
include_directories(include)
install(DIRECTORY include/lightstep DESTINATION include)

set(COLLECTOR_PROTO ${PROTO_PATH}/collector.proto)
set(LIGHTSTEP_CARRIER_PROTO ${PROTO_PATH}/lightstep_carrier.proto)
set(GENERATED_PROTOBUF_PATH ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(COLLECTOR_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/collector.pb.cc)
set(COLLECTOR_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/collector.pb.h)
set(COLLECTOR_GRPC_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/collector.grpc.pb.cc)
set(COLLECTOR_GRPC_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/collector.grpc.pb.h)

set(LIGHTSTEP_CARRIER_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/lightstep_carrier.pb.cc)
set(LIGHTSTEP_CARRIER_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/lightstep_carrier.pb.h)

add_custom_command(
    OUTPUT ${COLLECTOR_PB_H_FILE}
           ${COLLECTOR_PB_CPP_FILE}
           ${COLLECTOR_GRPC_PB_H_FILE}
           ${COLLECTOR_GRPC_PB_CPP_FILE}
           ${LIGHTSTEP_CARRIER_PB_H_FILE}
           ${LIGHTSTEP_CARRIER_PB_CPP_FILE}
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS "--proto_path=${PROTO_PATH}"
         "--cpp_out=${GENERATED_PROTOBUF_PATH}"
         "${COLLECTOR_PROTO}"
         "${LIGHTSTEP_CARRIER_PROTO}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS "--proto_path=${PROTO_PATH}"
         "--grpc_out=${GENERATED_PROTOBUF_PATH}"
         "--plugin=protoc-gen-grpc=/usr/local/bin/GRPC_CPP_PLUGIN"
         "${COLLECTOR_PROTO}"
    )
include_directories(SYSTEM ${GENERATED_PROTOBUF_PATH})

add_library(lightstep_protobuf OBJECT ${COLLECTOR_PB_CPP_FILE}
                                      ${COLLECTOR_GRPC_PB_CPP_FILE}
                                      ${COLLECTOR_CARRIER_PB_CPP_FILE}
                                      ${LIGHTSTEP_CARRIER_PB_CPP_FILE})

set(LIGHTSTEP_SRCS src/utility.cpp
                   src/propagation.cpp
                   src/binary_carrier.cpp
                   src/grpc_transporter.cpp
                   src/report_builder.cpp
                   src/rpc_recorder.cpp
                   src/lightstep_span_context.cpp
                   src/lightstep_span.cpp
                   src/lightstep_tracer_impl.cpp
                   src/tracer.cpp)

add_library(lightstep_tracer $<TARGET_OBJECTS:lightstep_protobuf>
                             ${LIGHTSTEP_SRCS})
target_compile_options(lightstep_tracer PUBLIC ${WARNING_CXX_FLAGS})
if (CLANG_TIDY_EXE)
  set_target_properties(lightstep_tracer PROPERTIES
                                         CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

install(TARGETS lightstep_tracer
        LIBRARY DESTINATION lib 
        ARCHIVE DESTINATION lib)

target_link_libraries(lightstep_tracer ${GRPC_LIBRARIES}
                                       ${GRPCPP_LIBRARIES})

if (WITH_ASAN)
  add_library(lightstep_tracer_asan $<TARGET_OBJECTS:lightstep_protobuf>
                                    ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer_asan PUBLIC ${ASAN_CXX_FLAGS})
  target_link_libraries(lightstep_tracer_asan ${GRPC_LIBRARIES}
                                              ${GRPCPP_LIBRARIES})
  set_target_properties(lightstep_tracer_asan 
    PROPERTIES LINK_FLAGS "${ASAN_LD_FLAGS}")
endif()

if (WITH_TSAN)
  add_library(lightstep_tracer_tsan $<TARGET_OBJECTS:lightstep_protobuf>
                                    ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer_tsan PUBLIC ${TSAN_CXX_FLAGS})
  target_link_libraries(lightstep_tracer_tsan ${GRPC_LIBRARIES}
                                              ${GRPCPP_LIBRARIES})
  set_target_properties(lightstep_tracer_tsan 
    PROPERTIES LINK_FLAGS "${TSAN_LD_FLAGS}")
endif()

add_subdirectory(test)
add_subdirectory(example)
