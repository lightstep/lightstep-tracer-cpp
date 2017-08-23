#pragma once

#include <google/protobuf/message.h>
#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
class Transporter {
 public:
  virtual ~Transporter() = default;
};

class SyncTransporter : public Transporter {
 public:
  virtual opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) = 0;
};

class AsyncTransporter : public Transporter {
 public:
  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    void (*on_success)(void* context),
                    void (*on_failure)(std::error_code error, void* context),
                    void* context) = 0;
};
}  // namespace lightstep
