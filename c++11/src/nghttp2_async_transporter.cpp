#include "nghttp2_async_transporter.h"

namespace lightstep {
void Nghttp2AsyncTransporter::OnTimeout() noexcept {
  if (on_failure_ == nullptr) {
    return;
  }
  on_failure_(std::make_error_code(std::errc::timed_out), context_);
}

void Nghttp2AsyncTransporter::Send(
    const google::protobuf::Message& request,
    google::protobuf::Message& response, void (*on_success)(void* context),
    void (*on_failure)(std::error_code error, void* context), void* context) {
  on_success_ = on_success;
  on_failure_ = on_failure;
  context_ = context;
}
} // namespace lightstep
