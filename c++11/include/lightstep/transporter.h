#pragma once

#include <google/protobuf/message.h>
#include <opentracing/util.h>
#include <functional>

namespace lightstep {
class AsyncTransporter {
 public:
  virtual ~AsyncTransporter() = default;

  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    std::function<void()> success_callback,
                    std::function<void(std::error_code)> fail_callback) = 0;
};
}  // namespace lightstep
