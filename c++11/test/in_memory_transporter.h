#pragma once

#include <iostream>
#include <vector>
#include "../src/transporter.h"

namespace lightstep {
class InMemoryTransporter : public Transporter {
 public:
  virtual opentracing::expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) noexcept override {
    collector::ReportResponse response;
    spans_.reserve(spans_.size() + report.spans_size());
    for (auto& span : report.spans()) {
      spans_.push_back(span);
    }
    return response;
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

 private:
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
