#include "in_memory_async_transporter.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Send
//------------------------------------------------------------------------------
void InMemoryAsyncTransporter::Send(const google::protobuf::Message& request,
                                    google::protobuf::Message& response,
                                    AsyncTransporter::Callback& callback) {
  active_request_ = &request;
  active_response_ = &response;
  active_callback_ = &callback;
}

//------------------------------------------------------------------------------
// Write
//------------------------------------------------------------------------------
void InMemoryAsyncTransporter::Write() {
  if (active_request_ == nullptr || active_response_ == nullptr ||
      active_callback_ == nullptr) {
    std::cerr << "No context, success callback, or request\n";
    std::terminate();
  }
  const collector::ReportRequest& report =
      dynamic_cast<const collector::ReportRequest&>(*active_request_);
  reports_.push_back(report);

  spans_.reserve(spans_.size() + report.spans_size());
  for (auto& span : report.spans()) {
    spans_.push_back(span);
  }

  active_response_->CopyFrom(
      *Transporter::MakeCollectorResponse(*active_request_));
  active_callback_->OnSuccess();
}

//------------------------------------------------------------------------------
// Fail
//------------------------------------------------------------------------------
void InMemoryAsyncTransporter::Fail(std::error_code error) {
  if (active_callback_ == nullptr) {
    std::cerr << "No context or failure callback\n";
    std::terminate();
  }

  active_callback_->OnFailure(error);
}
}  // namespace lightstep
