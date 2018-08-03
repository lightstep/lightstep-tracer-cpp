#pragma once

#include <opentracing/span.h>
#include <opentracing/string_view.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include "propagation.h"
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
class LightStepSpanContext : public opentracing::SpanContext {
 public:
  LightStepSpanContext() = default;

  LightStepSpanContext(
      uint64_t trace_id, uint64_t span_id,
      std::unordered_map<std::string, std::string>&& baggage) noexcept;

  LightStepSpanContext(uint64_t trace_id, uint64_t span_id,
                       BaggageMap&& baggage) noexcept;

  LightStepSpanContext(
      uint64_t trace_id, uint64_t span_id, bool sampled,
      std::unordered_map<std::string, std::string>&& baggage) noexcept;

  LightStepSpanContext(uint64_t trace_id, uint64_t span_id, bool sampled,
                       BaggageMap&& baggage) noexcept;

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
    return InjectSpanContext(propagation_options, writer, this->trace_id(),
                             this->span_id(), sampled_, data_.baggage());
  }

  template <class Carrier>
  opentracing::expected<bool> Extract(
      const PropagationOptions& propagation_options, Carrier& reader) {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    uint64_t trace_id, span_id;
    BaggageMap baggage;
    auto result = ExtractSpanContext(propagation_options, reader, trace_id,
                                     span_id, sampled_, baggage);
    if (!result) {
      return result;
    }
    data_.set_trace_id(trace_id);
    data_.set_span_id(span_id);
    *data_.mutable_baggage() = std::move(baggage);

    return result;
  }

  uint64_t trace_id() const noexcept { return data_.trace_id(); }
  uint64_t span_id() const noexcept { return data_.span_id(); }

  bool sampled() const noexcept;
  void set_sampled(bool sampled) noexcept;

 private:
  mutable std::mutex mutex_;
  collector::SpanContext data_;
  bool sampled_ = true;
};

bool operator==(const LightStepSpanContext& lhs,
                const LightStepSpanContext& rhs);

inline bool operator!=(const LightStepSpanContext& lhs,
                       const LightStepSpanContext& rhs) {
  return !(lhs == rhs);
}
}  // namespace lightstep
