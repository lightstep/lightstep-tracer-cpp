add_executable(upload_benchmark upload_benchmark.cpp 
                                dummy_satellite.cpp
                                grpc_dummy_satellite.cpp
                                stream_dummy_satellite.cpp
                                stream_session.cpp
                                upload_benchmark.cpp
                                upload_benchmark_report.cpp
                                main.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
            -Wno-missing-variable-declarations)
endif()

target_link_libraries(upload_benchmark lightstep_tracer
                                       ${LIGHTSTEP_LINK_LIBRARIES}
                                       gflags
                                       $<TARGET_OBJECTS:upload_benchmark_configuration>)

if (BUILD_TESTING)
  add_test(upload_benchmark_rpc_test upload_benchmark 
                  --recorder_type rpc 
                  --max_buffered_spans 100 
                  --num_threads 1 
                  --num_spans 1000 
                  --max_spans_per_second 1000)
add_test(upload_benchmark_rpc_threaded_test upload_benchmark 
                  --recorder_type rpc 
                  --max_buffered_spans 100 
                  --num_threads 2 
                  --num_spans 1000 
                  --max_spans_per_second 1000)
add_test(upload_benchmark_stream_test upload_benchmark 
                  --recorder_type stream 
                  --max_buffered_spans 100 
                  --num_threads 1 
                  --num_spans 1000 
                  --max_spans_per_second 1000)
add_test(upload_benchmark_stream_threaded_test upload_benchmark 
                  --recorder_type stream 
                  --max_buffered_spans 100 
                  --num_threads 2 
                  --num_spans 1000 
                  --max_spans_per_second 1000)
endif()
