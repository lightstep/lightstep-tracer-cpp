add_executable(tracer_test tracer_test.cpp)
target_link_libraries(tracer_test lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
add_test(tracer_test tracer_test)

add_executable(propagation_test propagation_test.cpp)
target_link_libraries(propagation_test lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
add_test(propagation_test propagation_test)
