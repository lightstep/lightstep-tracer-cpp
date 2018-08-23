#pragma once

#include <opentracing/span.h>
#include <opentracing/string_view.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include "lightstep-tracer-common/collector.pb.h"
#include "propagation.h"

namespace lightstep {
class LightStepSpanContext : public opentracing::SpanContext {
 public:
  virtual bool sampled() const noexcept = 0;

  virtual uint64_t trace_id() const noexcept = 0;

  virtual uint64_t span_id() const noexcept = 0;

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      std::ostream& writer) const = 0;

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::TextMapWriter& writer) const = 0;

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::HTTPHeadersWriter& writer) const = 0;
};

bool operator==(const LightStepSpanContext& lhs,
                const LightStepSpanContext& rhs);

inline bool operator!=(const LightStepSpanContext& lhs,
                       const LightStepSpanContext& rhs) {
  return !(lhs == rhs);
}
}  // namespace lightstep
