#pragma once

#include <cstdint>
#include <string>

#include "common/noncopyable.h"
#include "network/event_base.h"

#include <event2/http.h>
#include <google/protobuf/message.h>

struct evhttp_connection;
struct evhttp_request;

namespace lightstep {
/**
 * Manages an http connection.
 */
class HttpConnection : private Noncopyable {
 public:
  HttpConnection(const char* address, uint16_t port);

  ~HttpConnection() noexcept;

  /**
   * Sends an http get request.
   * @param uri the uri to use for the request.
   * @return the contents of the response.
   */
  std::string Get(const char* uri);

  /**
   * Sends an http request and parses the response into a protobuf message.
   * @param uri the uri to use for the request.
   * @param response the protobuf to parse the response into.
   */
  void Get(const char* uri, google::protobuf::Message& response);

  /**
   * Sends an http post request and parses the response into a protobuf message.
   * @param uri the uri to use for the request.
   * @param request a protobuf message to serialize into the request body.
   * @param response the protobuf to parse the response into.
   */
  void Post(const char* uri, const google::protobuf::Message& request,
            google::protobuf::Message& response);

  /**
   * Sends an http post request and parses the response into a protobuf message.
   * @param uri the uri to use for the request.
   * @param request a string to use in the request body.
   * @param response the protobuf to parse the response into.
   */
  void Post(const char* uri, const std::string& content,
            google::protobuf::Message& response);

 private:
  EventBase event_base_;
  evhttp_connection* connection_;
  std::string result_;
  google::protobuf::Message* response_message_{nullptr};
  bool error_{false};

  evhttp_request* MakeRequest(evhttp_cmd_type command, const char* uri,
                              const std::string& content = {});

  static void OnCompleteRequest(evhttp_request* request,
                                void* context) noexcept;
};
}  // namespace lightstep
