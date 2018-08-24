add_executable(upload_benchmark upload_benchmark.cpp grpc_dummy_satellite.cpp stream_dummy_satellite.cpp
  stream_session.cpp)
target_link_libraries(upload_benchmark lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})

add_executable(span_operations_benchmark span_operations_benchmark.cpp)
target_link_libraries(span_operations_benchmark lightstep_tracer 
                                                ${LIGHTSTEP_LINK_LIBRARIES}
                                                benchmark::benchmark)

if (BUILD_TESTING)
  add_test(upload_benchmark_rpc_test upload_benchmark rpc 1 1000 1000)
  add_test(upload_benchmark_rpc_threaded_test upload_benchmark rpc 2 1000 1000)
  add_test(upload_benchmark_stream_test upload_benchmark stream 1 1000 1000)
  add_test(upload_benchmark_stream_threaded_test upload_benchmark stream 2 1000 1000)

  add_test(span_operations_benchmark_test span_operations_benchmark)
endif()
