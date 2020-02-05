#pragma once

#include <functional>
#include <vector>

#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep/transporter.h"

namespace lightstep {
/**
 * An AsyncTransporter that "transports" spans in to a container.
 */
class InMemoryAsyncTransporter final : public AsyncTransporter {
 public:
  explicit InMemoryAsyncTransporter(
      std::function<void()> on_span_buffer_full = {});

  /**
   * @return the ReportRequests that were transported.
   */
  const std::vector<collector::ReportRequest>& reports() const noexcept {
    return reports_;
  }

  /**
   * All the spans that were transported.
   */
  const std::vector<collector::Span>& spans() const noexcept { return spans_; }

  /**
   * Make the last Send succeed.
   */
  void Succeed() noexcept;

  /**
   * Make the last Send fail.
   */
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
}  // namespace lightstep
