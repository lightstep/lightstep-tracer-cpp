if (LIGHTSTEP_USE_GRPC)
  add_executable(tutorial tutorial.cpp)
  target_link_libraries(tutorial ${LIGHTSTEP_LIBRARY} ${LIGHTSTEP_LINK_LIBRARIES})
endif()
