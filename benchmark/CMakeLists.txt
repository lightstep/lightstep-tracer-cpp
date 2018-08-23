add_executable(upload_benchmark upload_benchmark.cpp grpc_dummy_satellite.cpp stream_dummy_satellite.cpp)
target_link_libraries(upload_benchmark lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})

add_executable(span_operations_benchmark span_operations_benchmark.cpp)
target_link_libraries(span_operations_benchmark lightstep_tracer 
                                                ${LIGHTSTEP_LINK_LIBRARIES}
                                                benchmark::benchmark)
