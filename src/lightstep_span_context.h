#pragma once

#include <opentracing/span.h>
#include <opentracing/string_view.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include "propagation.h"

namespace lightstep {
class LightStepSpanContextBase : public opentracing::SpanContext {
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

class LightStepSpanContext final : public LightStepSpanContextBase {
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

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      std::ostream& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::TextMapWriter& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::HTTPHeadersWriter& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  template <class Carrier>
  opentracing::expected<bool> Extract(
      const PropagationOptions& propagation_options, Carrier& reader) {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return ExtractSpanContext(propagation_options, reader, trace_id_, span_id_,
                              sampled_, baggage_);
  }

  uint64_t trace_id() const noexcept override { return trace_id_; }
  uint64_t span_id() const noexcept override { return span_id_; }

  bool sampled() const noexcept override;
  void set_sampled(bool sampled) noexcept;

 private:
  uint64_t trace_id_ = 0;
  uint64_t span_id_ = 0;

  mutable std::mutex mutex_;
  bool sampled_ = true;
  std::unordered_map<std::string, std::string> baggage_;

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return InjectSpanContext(propagation_options, writer, trace_id_, span_id_,
                             sampled_, baggage_);
  }
};

bool operator==(const LightStepSpanContextBase& lhs,
                const LightStepSpanContextBase& rhs);

inline bool operator!=(const LightStepSpanContextBase& lhs,
                       const LightStepSpanContextBase& rhs) {
  return !(lhs == rhs);
}
}  // namespace lightstep
