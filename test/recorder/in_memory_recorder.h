#pragma once

#include "recorder/recorder.h"

#include <mutex>
#include <vector>

namespace lightstep {
// InMemoryRecorder is used for testing only.
class InMemoryRecorder final : public Recorder {
 public:
  std::vector<collector::Span> spans() const;

  size_t size() const;

  collector::Span top() const;

  // Recorder
  void RecordSpan(const collector::Span& span) noexcept override;

  void RecordSpan(std::unique_ptr<SerializationChain>&& span) noexcept override;

  const LightStepTracerOptions& tracer_options() const noexcept { return tracer_options_; }
 private:
  mutable std::mutex mutex_;
  std::vector<collector::Span> spans_;
  LightStepTracerOptions tracer_options_;
};
}  // namespace lightstep
