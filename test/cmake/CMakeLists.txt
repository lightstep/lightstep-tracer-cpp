if (WITH_DYNAMIC_LOAD AND BUILD_SHARED_LIBS)
  add_executable(load_tracer_test load_tracer_test.cpp)
  target_link_libraries(load_tracer_test ${OPENTRACING_LIBRARY}) 
  add_test(load_tracer_test load_tracer_test
    ${CMAKE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lightstep_tracer${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
