set(PROTO_PATH "${CMAKE_SOURCE_DIR}/lightstep-tracer-common")

if( NOT EXISTS "${PROTO_PATH}/.git" )
  execute_process(COMMAND git submodule update --init --recursive
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

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

if (LS_WITH_GRPC)
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
                                        ${LIGHTSTEP_CARRIER_PB_CPP_FILE})
else()
  add_custom_command(
      OUTPUT ${COLLECTOR_PB_H_FILE}
             ${COLLECTOR_PB_CPP_FILE}
             ${LIGHTSTEP_CARRIER_PB_H_FILE}
             ${LIGHTSTEP_CARRIER_PB_CPP_FILE}
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS "--proto_path=${PROTO_PATH}"
           "--cpp_out=${GENERATED_PROTOBUF_PATH}"
           "${COLLECTOR_PROTO}"
           "${LIGHTSTEP_CARRIER_PROTO}"
      )
  include_directories(SYSTEM ${GENERATED_PROTOBUF_PATH})
  
  add_library(lightstep_protobuf OBJECT ${COLLECTOR_PB_CPP_FILE}
                                        ${LIGHTSTEP_CARRIER_PB_CPP_FILE})
endif()
