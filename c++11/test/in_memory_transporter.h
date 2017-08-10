#pragma once

#include <collector.pb.h>
#include <lightstep/transporter.h>
#include <exception>
#include <iostream>
#include <vector>

namespace lightstep {
class InMemoryTransporter : public SyncTransporter {
 public:
  opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) override {
    if (should_throw_) {
      throw std::runtime_error{"should_throw_ == true"};
    }
    const collector::ReportRequest& report =
        static_cast<const collector::ReportRequest&>(request);
    spans_.reserve(spans_.size() + report.spans_size());
    for (auto& span : report.spans()) {
      spans_.push_back(span);
    }
    return {};
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

  void set_should_throw(bool value) { should_throw_ = value; }

 private:
  bool should_throw_ = false;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
