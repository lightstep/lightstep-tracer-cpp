find_library(LIBEVENT_LIB event)

if (LIBEVENT_LIB)
  find_path(LIBEVENT_INCLUDE NAMES event2/event.h)
  include_directories(SYSTEM ${LIBEVENT_INCLUDE})
  add_executable(qa-server qa_bot.cpp
                           qa_session.cpp
                           qa_server.cpp)
  target_link_libraries(qa-server ${LIBEVENT_LIB} lightstep_tracer ${OPENTRACING_LIB} ${PROTOBUF_LIBRARIES})
  add_executable(client client.cpp)
endif()
