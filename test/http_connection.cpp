#include "test/http_connection.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <event2/buffer.h>
#include <event2/http_struct.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ReadMessage
//--------------------------------------------------------------------------------------------------
static void ReadMessage(evbuffer& buffer, google::protobuf::Message& message) {
  std::string s;
  s.resize(evbuffer_get_length(&buffer));
  evbuffer_copyout(&buffer, static_cast<void*>(&s[0]), s.size());
  auto rcode = message.ParseFromString(s);
  if (!rcode) {
    std::cerr << "ParseFromString failed\n";
    std::terminate();
  }
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
HttpConnection::HttpConnection(const char* address, uint16_t port) {
  connection_ = evhttp_connection_base_new(event_base_.libevent_handle(),
                                           nullptr, address, port);
  if (connection_ == nullptr) {
    std::cerr << "evhttp_connection_base_new failed\n";
    std::terminate();
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
HttpConnection::~HttpConnection() noexcept {
  evhttp_connection_free(connection_);
}

//--------------------------------------------------------------------------------------------------
// Get
//--------------------------------------------------------------------------------------------------
void HttpConnection::Get(const char* uri, google::protobuf::Message& response) {
  MakeRequest(EVHTTP_REQ_POST, uri);
  response_message_ = &response;
  event_base_.Dispatch();
  if (error_) {
    std::cerr << "Get failed\n";
    std::terminate();
  }
}

//--------------------------------------------------------------------------------------------------
// Post
//--------------------------------------------------------------------------------------------------
void HttpConnection::Post(const char* uri,
                          const google::protobuf::Message& request,
                          google::protobuf::Message& response) {
  std::string content;
  request.SerializeToString(&content);
  MakeRequest(EVHTTP_REQ_POST, uri, content);
  response_message_ = &response;
  event_base_.Dispatch();
  if (error_) {
    std::cerr << "Post failed\n";
    std::terminate();
  }
}

void HttpConnection::Post(const char* uri, const std::string& content,
                          google::protobuf::Message& response) {
  MakeRequest(EVHTTP_REQ_POST, uri, content);
  response_message_ = &response;
  event_base_.Dispatch();
  if (error_) {
    std::cerr << "Post failed\n";
    std::terminate();
  }
}

//--------------------------------------------------------------------------------------------------
// MakeRequest
//--------------------------------------------------------------------------------------------------
evhttp_request* HttpConnection::MakeRequest(evhttp_cmd_type command,
                                            const char* uri,
                                            const std::string& content) {
  auto request = evhttp_request_new(HttpConnection::OnCompleteRequest,
                                    static_cast<void*>(this));
  if (request == nullptr) {
    std::cerr << "evhttp_request_new failed\n";
    std::terminate();
  }
  auto headers = evhttp_request_get_output_headers(request);
  auto rcode = evhttp_add_header(headers, "Host", "localhost");
  if (rcode != 0) {
    std::cerr << "evhttp_add_header failed\n";
    std::terminate();
  }
  auto output_buffer = evhttp_request_get_output_buffer(request);
  rcode = evbuffer_add(output_buffer, static_cast<const void*>(content.data()),
                       content.size());
  if (rcode != 0) {
    std::cerr << "evbuffer_add failure\n";
    std::terminate();
  }
  rcode = evhttp_make_request(connection_, request, command, uri);
  if (rcode != 0) {
    std::cerr << "evhttp_make_request failed\n";
    std::terminate();
  }
  return request;
}

//--------------------------------------------------------------------------------------------------
// OnCompleteRequest
//--------------------------------------------------------------------------------------------------
void HttpConnection::OnCompleteRequest(evhttp_request* request,
                                       void* context) noexcept {
  auto self = static_cast<HttpConnection*>(context);
  self->error_ = request == nullptr || request->response_code != 200;
  ReadMessage(*evhttp_request_get_input_buffer(request),
              *self->response_message_);
  self->response_message_ = nullptr;
  self->event_base_.LoopBreak();
}
}  // namespace lightstep
