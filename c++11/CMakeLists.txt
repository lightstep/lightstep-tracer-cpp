set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
include_directories(${GENERATED_PROTOBUF_PATH})

add_library(lightstep_tracer ${COLLECTOR_PB_CPP_FILE}
                             ${COLLECTOR_GRPC_PB_CPP_FILE}
                             ${COLLECTOR_CARRIER_PB_CPP_FILE}
                             src/utility.cpp
                             src/tracer.cpp
                             src/propagation.cpp
                             src/rpc_recorder.cpp)

install(TARGETS lightstep_tracer
        LIBRARY DESTINATION lib 
        ARCHIVE DESTINATION lib)

target_link_libraries(lightstep_tracer ${GRPC_LIBRARIES}
                                       ${GRPCPP_LIBRARIES})

add_subdirectory(test)
add_subdirectory(example)
