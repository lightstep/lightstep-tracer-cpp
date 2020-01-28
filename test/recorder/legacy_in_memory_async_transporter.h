#pragma once

#include <exception>
#include <iostream>
#include <mutex>
#include <vector>

#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep/transporter.h"

namespace lightstep {
class LegacyInMemoryAsyncTransporter final : public LegacyAsyncTransporter {
 public:
  void Write();

  void Fail(std::error_code error);

  const std::vector<collector::ReportRequest>& reports() const {
    return reports_;
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

  void set_should_disable(bool value) { should_disable_ = value; }

  // LegacyAsyncTransporter
  void Send(const google::protobuf::Message& request,
            google::protobuf::Message& response,
            LegacyAsyncTransporter::Callback& callback) override;

 private:
  bool should_disable_ = false;
  const google::protobuf::Message* active_request_;
  google::protobuf::Message* active_response_;
  LegacyAsyncTransporter::Callback* active_callback_;
  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
