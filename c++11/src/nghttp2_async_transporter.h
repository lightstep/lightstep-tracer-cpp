#pragma once

#include <lightstep/transporter.h>

namespace lightstep {
// This still needs to be filled in, but shows what the interface for a
// LightStep-provided async transporter might look like.
class Nghttp2AsyncTransporter : public LightStepAsyncTransporter {
 public:
  int file_descriptor() const noexcept override {
    // Since this isn't implemented yet, so return a dummy file descriptor for
    // now.
    return -1;
  }

  void OnRead() noexcept override {}
  void OnWrite() noexcept override {}
  void OnTimeout() noexcept override;
  void Send(const google::protobuf::Message& request,
            google::protobuf::Message& response,
            void (*on_success)(void* context),
            void (*on_failure)(std::error_code error, void* context),
            void* context) override;

 private:
  void (*on_success_)(void*) = nullptr;
  void (*on_failure_)(std::error_code, void*) = nullptr;
  void* context_ = nullptr;
};
}  // namespace lightstep
