add_executable(tracer_test tracer_test.cpp)
target_link_libraries(tracer_test lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
add_test(tracer_test tracer_test)

add_executable(propagation_test propagation_test.cpp)
target_link_libraries(propagation_test lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
add_test(propagation_test propagation_test)

add_executable(recorder_test recorder_test.cpp span_generator.cpp)
target_link_libraries(recorder_test lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
add_test(recorder_test recorder_test)
