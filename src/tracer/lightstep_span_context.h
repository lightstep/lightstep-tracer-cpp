#pragma once

#include <opentracing/span.h>
#include <opentracing/string_view.h>
#include <mutex>
#include <string>
#include <unordered_map>

#include "tracer/propagation/propagation.h"

namespace lightstep {
class LightStepSpanContext : public opentracing::SpanContext {
 public:
  virtual uint64_t trace_id_high() const noexcept = 0;

  virtual uint64_t trace_id_low() const noexcept = 0;

  virtual uint64_t span_id() const noexcept = 0;

  bool sampled() const noexcept {
    return IsTraceFlagSet<SampledFlagMask>(this->trace_flags());
  }

  virtual uint8_t trace_flags() const noexcept = 0;

  virtual opentracing::string_view trace_state() const noexcept = 0;

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
