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

  void RecordSpan(Fragment header_fragment,
                  std::unique_ptr<ChainedStream>&& span) noexcept override;

 private:
  mutable std::mutex mutex_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
