#pragma once

#include <vector>
#include <functional>

#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep/transporter.h"

namespace lightstep {
class InMemoryAsyncTransporter final : public AsyncTransporter {
 public:
  explicit InMemoryAsyncTransporter(
      std::function<void()> on_span_buffer_full = {});

  const std::vector<collector::ReportRequest>& reports() const noexcept {
    return reports_;
  }

  const std::vector<collector::Span>& spans() const noexcept { return spans_; }

  void Succeed() noexcept;

  void Fail() noexcept;

   // AsyncTranspoter
  void OnSpanBufferFull() noexcept override;

  void Send(std::unique_ptr<BufferChain>&& message,
                    Callback& callback) noexcept override;
 private:
  std::function<void()> on_span_buffer_full_;

  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;

  Callback* active_callback_{nullptr};
  std::unique_ptr<BufferChain> active_message_;
};
} // namespace lightstep
