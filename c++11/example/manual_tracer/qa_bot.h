#pragma once

#include <event2/listener.h>

class QABot {
 public:
  explicit QABot(event_base* base);

  ~QABot();

 private:
  evconnlistener* listener_;

  static void AcceptConnectionCallback(evconnlistener* listener,
                                       evutil_socket_t socketfd,
                                       sockaddr* address, int socket_len,
                                       void* context);

  static void AcceptErrorCallback(evconnlistener* listener, void* context);
};
