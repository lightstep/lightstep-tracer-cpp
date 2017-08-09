#pragma once

#include <exception>
#include <iostream>
#include <vector>
#include "../src/transporter.h"

namespace lightstep {
class InMemoryTransporter : public Transporter {
 public:
  virtual opentracing::expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) override {
    if (should_throw_) {
      throw std::runtime_error{"should_throw_ == true"};
    }
    collector::ReportResponse response;
    spans_.reserve(spans_.size() + report.spans_size());
    for (auto& span : report.spans()) {
      spans_.push_back(span);
    }
    return response;
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

  void set_should_throw(bool value) { should_throw_ = value; }

 private:
  bool should_throw_ = false;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
