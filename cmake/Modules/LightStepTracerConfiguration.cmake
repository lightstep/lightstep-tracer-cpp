set(PROTO_PATH "${CMAKE_SOURCE_DIR}/lightstep-tracer-configuration")

set(TRACER_CONFIGURATION_PROTO ${PROTO_PATH}/tracer_configuration.proto)
set(GENERATED_PROTOBUF_PATH ${CMAKE_BINARY_DIR}/generated/lightstep-tracer-configuration)
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(TRACER_CONFIGURATION_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/tracer_configuration.pb.cc)
set(TRACER_CONFIGURATION_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/tracer_configuration.pb.h)

add_custom_command(
  OUTPUT ${TRACER_CONFIGURATION_PB_CPP_FILE}
         ${TRACER_CONFIGURATION_PB_H_FILE}
 COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
 ARGS "--proto_path=${PROTO_PATH}"
      "--cpp_out=${GENERATED_PROTOBUF_PATH}"
      ${TRACER_CONFIGURATION_PROTO}
)

include_directories(SYSTEM ${GENERATED_PROTOBUF_PATH})
include_directories(SYSTEM ${GENERATED_PROTOBUF_PATH}/../)

add_library(lightstep_tracer_configuration OBJECT ${TRACER_CONFIGURATION_PB_CPP_FILE})

if (BUILD_SHARED_LIBS)
  set_property(TARGET lightstep_tracer_configuration PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
