macro(_lightstep_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_link_libraries(${TEST_NAME} lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
  target_compile_options(${TEST_NAME} PUBLIC "-g")
  add_test(${TEST_NAME} ${TEST_NAME} ${${TEST_NAME}_opts})
endmacro()

_lightstep_test(tracer_test tracer_test.cpp 
                            utility.cpp)
_lightstep_test(utility_test utility_test.cpp)
_lightstep_test(logger_test logger_test.cpp)
_lightstep_test(propagation_test propagation_test.cpp)
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
