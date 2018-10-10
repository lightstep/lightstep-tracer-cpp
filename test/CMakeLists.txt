
macro(_lightstep_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_link_libraries(${TEST_NAME} lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
  target_compile_options(${TEST_NAME} PUBLIC "-g")
  add_test(${TEST_NAME} ${TEST_NAME} ${${TEST_NAME}_opts})
endmacro()

_lightstep_test(tracer_test tracer_test.cpp 
                            utility.cpp)
_lightstep_test(address_info_test address_info_test.cpp)
_lightstep_test(bipart_memory_stream_test bipart_memory_stream_test.cpp)
_lightstep_test(utility_test utility_test.cpp)
_lightstep_test(circular_buffer_test circular_buffer_test.cpp)
_lightstep_test(packet_header_test packet_header_test.cpp)
_lightstep_test(message_buffer_test message_buffer_test.cpp)
_lightstep_test(atomic_bit_set_test atomic_bit_set_test.cpp)
_lightstep_test(logger_test logger_test.cpp)

if (WITH_GRPC)
  add_subdirectory(dummy_satellite)
  _lightstep_test(satellite_connection_test satellite_connection_test.cpp)
  target_link_libraries(satellite_connection_test dummy_satellite)
endif()

_lightstep_test(socket_test socket_test.cpp)
_lightstep_test(fork_id_test fork_id_test.cpp)
_lightstep_test(propagation_test propagation_test.cpp)
_lightstep_test(stream_recorder_test stream_recorder_test.cpp)
_lightstep_test(satellite_stream_transporter_test satellite_stream_transporter_test.cpp)
_lightstep_test(auto_recorder_test auto_recorder_test.cpp
                                   utility.cpp
                                   in_memory_sync_transporter.cpp
                                   testing_condition_variable_wrapper.cpp)
_lightstep_test(manual_recorder_test manual_recorder_test.cpp
                                     utility.cpp
                                     in_memory_async_transporter.cpp
                                     testing_condition_variable_wrapper.cpp)
if (WITH_DYNAMIC_LOAD AND BUILD_SHARED_LIBS)
  set(dynamic_load_test_opts --lightstep_library 
    ${CMAKE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lightstep_tracer${CMAKE_SHARED_LIBRARY_SUFFIX})
  _lightstep_test(dynamic_load_test dynamic_load_test.cpp
                                    in_memory_collector.cpp)
endif()
