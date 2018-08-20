add_executable(span_operations_benchmark span_operations_benchmark.cpp)
target_link_libraries(span_operations_benchmark lightstep_tracer 
                                                ${LIGHTSTEP_LINK_LIBRARIES}
                                                benchmark::benchmark)
