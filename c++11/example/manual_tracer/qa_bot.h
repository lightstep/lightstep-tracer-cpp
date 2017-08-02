#pragma once

#include <event2/listener.h>
#include <opentracing/tracer.h>

class QABot {
 public:
  QABot(std::shared_ptr<opentracing::Tracer>&& tracer, event_base* base);

  ~QABot();

 private:
  std::shared_ptr<opentracing::Tracer> tracer_;
  evconnlistener* listener_;

  static void AcceptConnectionCallback(evconnlistener* listener,
                                       evutil_socket_t socketfd,
                                       sockaddr* address, int socket_len,
                                       void* context);

  static void AcceptErrorCallback(evconnlistener* listener, void* context);
};
