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
// Get
//--------------------------------------------------------------------------------------------------
void HttpConnection::Get(const char* uri, google::protobuf::Message& response) {
  MakeRequest(EVHTTP_REQ_POST, uri);
  response_message_ = &response;
  event_base_.Dispatch();
  if (error_) {
    throw std::runtime_error{"Get failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// Post
//--------------------------------------------------------------------------------------------------
void HttpConnection::Post(const char* uri,
                          const google::protobuf::Message& request,
                          google::protobuf::Message& response) {
  auto libevent_request = MakeRequest(EVHTTP_REQ_POST, uri);
  WriteMessage(request, *evhttp_request_get_output_buffer(libevent_request));
  response_message_ = &response;
  event_base_.Dispatch();
  if (error_) {
    throw std::runtime_error{"Post failed"};
  }
}

void HttpConnection::Post(const char* uri, const char* data, size_t size,
                          google::protobuf::Message& response) {
  auto request = MakeRequest(EVHTTP_REQ_POST, uri);
  auto output_buffer = evhttp_request_get_output_buffer(request);
  auto rcode = evbuffer_add(output_buffer, static_cast<const void*>(data), size);
  if (rcode != 0) {
    throw std::runtime_error{"evbuffer_add failure"};
  }
  response_message_ = &response;
  event_base_.Dispatch();
  if (error_) {
    throw std::runtime_error{"Post failed"};
  }
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
  auto headers = evhttp_request_get_output_headers(request);
  auto rcode = evhttp_add_header(headers, "Host", "localhost");
  if (rcode != 0) {
    evhttp_request_free(request);
    throw std::runtime_error{"evhttp_add_header failed"};
  }
  rcode = evhttp_make_request(connection_, request, command, uri);
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
  ReadMessage(*evhttp_request_get_input_buffer(request), *self->response_message_);
  self->response_message_ = nullptr;
  self->event_base_.LoopBreak();
}
} // namespace lightstep
