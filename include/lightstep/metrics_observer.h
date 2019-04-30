#pragma once

namespace lightstep {
// MetricsObserver can be used to track LightStep tracer events.
class MetricsObserver {
 public:
  MetricsObserver() noexcept = default;

  MetricsObserver(const MetricsObserver&) noexcept = default;

  MetricsObserver(MetricsObserver&&) noexcept = default;

  virtual ~MetricsObserver() = default;

  MetricsObserver& operator=(const MetricsObserver&) noexcept = default;
  MetricsObserver& operator=(MetricsObserver&&) noexcept = default;

  // OnSpansSent records spans transported.
  virtual void OnSpansSent(int /*num_spans*/) noexcept {}

  // OnSpansDropped records spans dropped.
  virtual void OnSpansDropped(int /*num_spans*/) noexcept {}

  // OnFlush records flush events by the recorder.
  virtual void OnFlush() noexcept {}
};
}  // namespace lightstep
