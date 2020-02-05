#pragma once

#include <memory>

#include "common/chained_stream.h"
#include "lightstep/buffer_chain.h"
#include "recorder/serialization/embedded_metrics_message.h"

namespace lightstep {
/**
 * Maintains a BufferChain for a collector::ReportRequest serialization.
 */
class ReportRequest final : public BufferChain {
 public:
  ReportRequest(std::shared_ptr<const std::string> header,
                int num_dropped_spans);

  /**
   * Add a serialized Span to the ReportRequest.
   * @param span the serialized span to add.
   */
  void AddSpan(std::unique_ptr<ChainedStream>&& span) noexcept;

  /**
   * @return the number of dropped spans recorded in the ReportRequest
   */
  int num_dropped_spans() const noexcept;

  /**
   * @return the number of spans in the ReportRequest
   */
  int num_spans() const noexcept { return num_spans_; }

  // BufferChain
  size_t num_fragments() const noexcept override {
    return static_cast<size_t>(num_fragments_);
  }

  size_t num_bytes() const noexcept override {
    return static_cast<size_t>(num_bytes_);
  }

  bool ForEachFragment(FragmentCallback callback, void* context) const override;

 private:
  std::shared_ptr<const std::string> header_;

  std::unique_ptr<const EmbeddedMetricsMessage> metrics_;
  std::pair<void*, int> metrics_fragment_;

  int num_bytes_{0};
  int num_spans_{0};
  int num_fragments_{0};
  std::unique_ptr<ChainedStream> spans_;
};
}  // namespace lightstep
