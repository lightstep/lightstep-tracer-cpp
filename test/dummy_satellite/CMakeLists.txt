add_library(dummy_satellite dummy_satellite.cpp
                            grpc_dummy_satellite.cpp
                            stream_dummy_satellite.cpp
                            stream_session.cpp)

target_link_libraries(dummy_satellite lightstep_tracer
                                       ${LIGHTSTEP_LINK_LIBRARIES})
