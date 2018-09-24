add_executable(span_operations_benchmark span_operations_benchmark.cpp)
target_link_libraries(span_operations_benchmark lightstep_tracer 
                                                ${LIGHTSTEP_LINK_LIBRARIES}
                                                benchmark::benchmark)

if (BUILD_TESTING)
  add_test(span_operations_benchmark_test span_operations_benchmark --benchmark_min_time=0)
endif()

add_subdirectory(upload)
