#pragma once

#include <google/protobuf/message.h>
#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
class AsyncTransporter {
 public:
  virtual ~AsyncTransporter() = default;

  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    void (*on_success)(void* context),
                    void (*on_failure)(std::error_code error, void* context),
                    void* context) = 0;
};

class LightStepAsyncTransporter : public AsyncTransporter {
 public:
  virtual int file_descriptor() const noexcept = 0;

  virtual void OnRead() noexcept = 0;
  virtual void OnWrite() noexcept = 0;
  virtual void OnTimeout() noexcept = 0;
};

std::unique_ptr<LightStepAsyncTransporter> MakeLightStepAsyncTransporter();
}  // namespace lightstep
