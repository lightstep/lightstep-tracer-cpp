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
            AsyncTransporter::Callback& callback) override {
    active_request_ = &request;
    active_callback_ = &callback;
  }

  void Write() {
    if (active_request_ == nullptr || active_callback_ == nullptr) {
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

    active_callback_->OnSuccess();
  }

  void Fail(std::error_code error) {
    if (active_callback_ == nullptr) {
      std::cerr << "No context or failure callback\n";
      std::terminate();
    }

    active_callback_->OnFailure(error);
  }

  const std::vector<collector::ReportRequest>& reports() const {
    return reports_;
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

 private:
  const google::protobuf::Message* active_request_;
  AsyncTransporter::Callback* active_callback_;
  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
