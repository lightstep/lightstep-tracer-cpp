#pragma once

#include <collector.pb.h>
#include <lightstep/transporter.h>
#include <exception>
#include <iostream>
#include <mutex>
#include <vector>

namespace lightstep {
class InMemorySyncTransporter : public SyncTransporter {
 public:
  opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& /*response*/) override {
    if (should_throw_) {
      throw std::runtime_error{"should_throw_ == true"};
    }
    const collector::ReportRequest& report =
        dynamic_cast<const collector::ReportRequest&>(request);
    reports_.push_back(report);

    spans_.reserve(spans_.size() + report.spans_size());
    for (auto& span : report.spans()) {
      spans_.push_back(span);
    }
    return {};
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

class InMemoryAsyncTransporter : public AsyncTransporter {
 public:
  void Send(const google::protobuf::Message& request,
            google::protobuf::Message& /*response*/,
            void (*on_success)(void* context),
            void (*on_failure)(std::error_code error, void* context),
            void* context) override {
    active_request_ = &request;
    active_context_ = context;
    on_success_ = on_success;
    on_failure_ = on_failure;
  }

  void Write() {
    if (active_request_ == nullptr || on_success_ == nullptr ||
        active_context_ == nullptr) {
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

    on_success_(active_context_);
  }

  void Fail(std::error_code error) {
    if (on_failure_ == nullptr || active_context_ == nullptr) {
      std::cerr << "No context or failure callback\n";
      std::terminate();
    }

    on_failure_(error, active_context_);
  }

  const std::vector<collector::ReportRequest>& reports() const {
    return reports_;
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

 private:
  const google::protobuf::Message* active_request_;
  void* active_context_;
  void (*on_success_)(void* context);
  void (*on_failure_)(std::error_code error, void* context);
  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
