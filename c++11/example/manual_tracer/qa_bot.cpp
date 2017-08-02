#include "qa_bot.h"
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <exception>
#include <iostream>
#include "config.h"
#include "qa_session.h"

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
QABot::QABot(std::shared_ptr<opentracing::Tracer>&& tracer, event_base* base) 
  : tracer_{std::move(tracer)}
{
  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(qabot_port);

  listener_ =
      evconnlistener_new_bind(base, &QABot::AcceptConnectionCallback, this,
                              LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                              (struct sockaddr*)&address, sizeof(address));
  if (!listener_) {
    std::cerr << "Listen failed!\n";
    std::terminate();
  }

  evconnlistener_set_error_cb(listener_, &QABot::AcceptErrorCallback);
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
QABot::~QABot() { evconnlistener_free(listener_); }

//------------------------------------------------------------------------------
// AcceptConnectionCallback
//------------------------------------------------------------------------------
void QABot::AcceptConnectionCallback(evconnlistener* listener,
                                     evutil_socket_t socketfd,
                                     sockaddr* /*address*/, int /*socket_len*/,
                                     void* context) {
  auto qa_bot = static_cast<QABot*>(context);
  auto base = evconnlistener_get_base(listener);
  auto session =
      new QASession{qa_bot->tracer_->StartSpan("QASession"), socketfd, base};
}

//------------------------------------------------------------------------------
// AcceptErrorCallback
//------------------------------------------------------------------------------
void QABot::AcceptErrorCallback(evconnlistener* listener, void* context) {
  auto* base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  std::cerr << "Listener Error: " << evutil_socket_error_to_string(err) << "\n";
  event_base_loopexit(base, nullptr);
}
