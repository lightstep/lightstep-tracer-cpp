add_executable(upload_benchmark upload_benchmark.cpp 
                                dummy_satellite.cpp
                                grpc_dummy_satellite.cpp
                                stream_dummy_satellite.cpp
                                stream_session.cpp
                                upload_benchmark.cpp
                                upload_benchmark_report.cpp
                                main.cpp)
target_link_libraries(upload_benchmark lightstep_tracer
                                       ${LIGHTSTEP_LINK_LIBRARIES}
                                       $<TARGET_OBJECTS:upload_benchmark_configuration>)

if (BUILD_TESTING)
  add_test(upload_benchmark_rpc_test upload_benchmark rpc 100 1 1000 1000)
  add_test(upload_benchmark_rpc_threaded_test upload_benchmark rpc 100 2 1000 1000)
  add_test(upload_benchmark_stream_test upload_benchmark stream 100 1 1000 1000)
  add_test(upload_benchmark_stream_threaded_test upload_benchmark stream 100 2 1000 1000)
endif()
