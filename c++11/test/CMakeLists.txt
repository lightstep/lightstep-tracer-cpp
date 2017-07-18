macro(_lightstep_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_link_libraries(${TEST_NAME} lightstep_tracer ${OPENTRACING_LIB} 
                        ${PROTOBUF_LIBRARIES})
  add_test(${TEST_NAME} ${TEST_NAME})

  if (WITH_ASAN)
    add_executable(${TEST_NAME}_asan ${ARGN})
    target_link_libraries(${TEST_NAME}_asan lightstep_tracer_asan 
                          ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
    target_compile_options(${TEST_NAME}_asan PUBLIC ${ASAN_CXX_FLAGS})
    set_target_properties(${TEST_NAME}_asan 
      PROPERTIES LINK_FLAGS "${ASAN_LD_FLAGS}")
    add_test(${TEST_NAME}_asan ${TEST_NAME}_asan)
  endif()

  if (WITH_TSAN)
    add_executable(${TEST_NAME}_tsan ${ARGN})
    target_link_libraries(${TEST_NAME}_tsan lightstep_tracer_tsan 
                          ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
    target_compile_options(${TEST_NAME}_tsan PUBLIC ${TSAN_CXX_FLAGS})
    set_target_properties(${TEST_NAME}_tsan 
      PROPERTIES LINK_FLAGS "${TSAN_LD_FLAGS}")
    add_test(${TEST_NAME}_tsan ${TEST_NAME}_tsan)
  endif()
endmacro()

_lightstep_test(tracer_test tracer_test.cpp)
_lightstep_test(propagation_test propagation_test.cpp)
_lightstep_test(recorder_test recorder_test.cpp span_generator.cpp)
