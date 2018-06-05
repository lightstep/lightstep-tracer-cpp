#pragma once

#include <opentracing/span.h>
#include <opentracing/string_view.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include "propagation.h"

namespace lightstep {
class LightStepSpanContext : public opentracing::SpanContext {
 public:
  LightStepSpanContext() = default;

  LightStepSpanContext(
      uint64_t trace_id, uint64_t span_id,
      std::unordered_map<std::string, std::string>&& baggage) noexcept;

  LightStepSpanContext(
      uint64_t trace_id, uint64_t span_id, bool sampled,
      std::unordered_map<std::string, std::string>&& baggage) noexcept;

  LightStepSpanContext(const LightStepSpanContext&) = delete;
  LightStepSpanContext(LightStepSpanContext&&) = delete;

  ~LightStepSpanContext() override = default;

  LightStepSpanContext& operator=(LightStepSpanContext&) = delete;
  LightStepSpanContext& operator=(LightStepSpanContext&& other) noexcept;

  void set_baggage_item(opentracing::string_view key,
                        opentracing::string_view value) noexcept;

  std::string baggage_item(opentracing::string_view key) const;

  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const override;

  template <class Carrier>
  opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return InjectSpanContext(propagation_options, writer, trace_id_, span_id_,
                             sampled_, baggage_);
  }

  template <class Carrier>
  opentracing::expected<bool> Extract(
      const PropagationOptions& propagation_options, Carrier& reader) {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return ExtractSpanContext(propagation_options, reader, trace_id_, span_id_,
                              sampled_, baggage_);
  }

  uint64_t trace_id() const noexcept { return trace_id_; }
  uint64_t span_id() const noexcept { return span_id_; }

  bool sampled() const noexcept;
  void set_sampled(bool sampled) noexcept;

 private:
  uint64_t trace_id_ = 0;
  uint64_t span_id_ = 0;

  mutable std::mutex mutex_;
  bool sampled_ = true;
  std::unordered_map<std::string, std::string> baggage_;
};

bool operator==(const LightStepSpanContext& lhs,
                const LightStepSpanContext& rhs);

inline bool operator!=(const LightStepSpanContext& lhs,
                       const LightStepSpanContext& rhs) {
  return !(lhs == rhs);
}
}  // namespace lightstep
