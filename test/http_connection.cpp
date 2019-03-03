#include "test/http_connection.h"

#include <stdexcept>
#include <string>

#include <event2/buffer.h>
#include <event2/http_struct.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// WriteMessage
//--------------------------------------------------------------------------------------------------
static void WriteMessage(const google::protobuf::Message& message, evbuffer& buffer) {
  std::string s;
  message.SerializeToString(&s);
  auto rcode =
      evbuffer_add(&buffer, static_cast<const void*>(s.data()), s.size());
  if (rcode != 0) {
    throw std::runtime_error{"evbuffer_add failure"};
  }
}

//--------------------------------------------------------------------------------------------------
// ReadMessage
//--------------------------------------------------------------------------------------------------
static void ReadMessage(evbuffer& buffer, google::protobuf::Message& message) {
  std::string s;
  s.resize(evbuffer_get_length(&buffer));
  evbuffer_copyout(&buffer, static_cast<void*>(&s[0]), s.size());
  auto rcode = message.ParseFromString(s);
  if (!rcode) {
    throw std::runtime_error{"ParseFromString failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
HttpConnection::HttpConnection(const char* address, uint16_t port) {
  connection_ = evhttp_connection_base_new(event_base_.libevent_handle(),
                                           nullptr, address, port);
  if (connection_ == nullptr) {
    throw std::runtime_error{"evhttp_connection_base_new failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
HttpConnection::~HttpConnection() noexcept {
  evhttp_connection_free(connection_);
}

//--------------------------------------------------------------------------------------------------
// Post
//--------------------------------------------------------------------------------------------------
void HttpConnection::Post(const char* uri,
                          const google::protobuf::Message& request,
                          google::protobuf::Message& response) {
  auto libevent_request = MakeRequest(EVHTTP_REQ_POST, uri);
  WriteMessage(request, *evhttp_request_get_output_buffer(libevent_request));
  event_base_.Dispatch();
  if (error_) {
    throw std::runtime_error{"Post failed"};
  }
  ReadMessage(*evhttp_request_get_input_buffer(libevent_request), response);
}

//--------------------------------------------------------------------------------------------------
// MakeRequest
//--------------------------------------------------------------------------------------------------
evhttp_request* HttpConnection::MakeRequest(evhttp_cmd_type command, const char* uri) {
  auto request = evhttp_request_new(HttpConnection::OnCompleteRequest,
                                    static_cast<void*>(this));
  if (request == nullptr) {
    throw std::runtime_error{"evhttp_request_new failed"};
  }
  auto rcode = evhttp_make_request(connection_, request, command, uri);
  if (rcode != 0) {
    throw std::runtime_error{"evhttp_make_request failed"};
  }
  return request;
}

//--------------------------------------------------------------------------------------------------
// OnCompleteRequest
//--------------------------------------------------------------------------------------------------
void HttpConnection::OnCompleteRequest(evhttp_request* request, void* context) noexcept {
  auto self = static_cast<HttpConnection*>(context);
  self->error_ = request == nullptr || request->response_code != 200;
  self->event_base_.LoopBreak();
}
} // namespace lightstep
