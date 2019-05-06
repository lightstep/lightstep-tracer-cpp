if (LIGHTSTEP_USE_STREAMING)
  add_executable(stream main.cpp)
  target_link_libraries(stream lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
endif()
