#pragma once

#include <exception>
#include <iostream>
#include <mutex>
#include <vector>
#include "../src/transporter.h"

namespace lightstep {
class InMemoryTransporter : public Transporter {
 public:
  virtual opentracing::expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) override {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (should_throw_) {
      throw std::runtime_error{"should_throw_ == true"};
    }
    reports_.push_back(report);

    spans_.reserve(spans_.size() + report.spans_size());
    for (auto& span : report.spans()) {
      spans_.push_back(span);
    }

    collector::ReportResponse response;
    return response;
  }

  std::vector<collector::Span> spans() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return spans_;
  }

  std::vector<collector::ReportRequest> reports() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return reports_;
  }

  void set_should_throw(bool value) {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    should_throw_ = value;
  }

 private:
  mutable std::mutex mutex_;
  bool should_throw_ = false;
  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
