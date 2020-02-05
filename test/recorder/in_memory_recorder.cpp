#include "test/recorder/in_memory_recorder.h"

#include <algorithm>
#include <exception>
#include <stdexcept>

#include "test/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// spans
//--------------------------------------------------------------------------------------------------
std::vector<collector::Span> InMemoryRecorder::spans() const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return spans_;
}

//--------------------------------------------------------------------------------------------------
// size
//--------------------------------------------------------------------------------------------------
size_t InMemoryRecorder::size() const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return spans_.size();
}

//--------------------------------------------------------------------------------------------------
// top
//--------------------------------------------------------------------------------------------------
collector::Span InMemoryRecorder::top() const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  if (spans_.empty()) {
    throw std::runtime_error("no spans");
  }
  return spans_.back();
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void InMemoryRecorder::RecordSpan(const collector::Span& span) noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  spans_.emplace_back(span);
}

void InMemoryRecorder::RecordSpan(
    Fragment /*header_fragment*/,
    std::unique_ptr<ChainedStream>&& span) noexcept {
  span->CloseOutput();
  auto serialization = ToString(*span);
  collector::Span protobuf_span;
  if (!protobuf_span.ParseFromString(serialization)) {
    std::cerr << "Failed to parse span\n";
    std::terminate();
  }
  std::lock_guard<std::mutex> lock_guard{mutex_};
  spans_.emplace_back(std::move(protobuf_span));
}
}  // namespace lightstep
