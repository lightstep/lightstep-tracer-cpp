#pragma once

#include <cstdint>

#include "common/noncopyable.h"
#include "network/event_base.h"

#include <google/protobuf/message.h>
#include <event2/http.h>

struct evhttp_connection;
struct evhttp_request;

namespace lightstep {
class HttpConnection : private Noncopyable {
 public:
   HttpConnection(const char* address, uint16_t port);

   ~HttpConnection() noexcept;

   void Get(const char* uri, google::protobuf::Message& response);

   void Post(const char* uri, const google::protobuf::Message& request,
             google::protobuf::Message& response);

   void Post(const char* uri, const char* data, size_t size,
             google::protobuf::Message& response);

  private:
   EventBase event_base_;
   evhttp_connection* connection_;
   google::protobuf::Message* response_message_{nullptr};
   bool error_{false};

   evhttp_request* MakeRequest(evhttp_cmd_type command, const char* uri);

   static void OnCompleteRequest(evhttp_request* request, void* context) noexcept;
};
} // namespace lightstep
