add_executable(tutorial tutorial.cpp)
target_link_libraries(tutorial lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
add_test(tutorial tutorial)

