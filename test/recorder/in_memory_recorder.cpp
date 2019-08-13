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
    std::unique_ptr<SerializationChain>&& span) noexcept {
  span->AddFraming();
  auto serialization = ToString(*span);
  auto header_last =
      std::find(serialization.begin(), serialization.end(), '\n') + 1;
  auto message_size = serialization.size() -
                      std::distance(serialization.begin(), header_last) - 2;
  collector::ReportRequest request;
  if (!request.ParseFromArray(static_cast<void*>(&*header_last),
                              message_size)) {
    std::cerr << "Failed to parse span\n";
    std::terminate();
  }
  if (request.spans().size() != 1) {
    std::cerr << "No span found\n";
    std::terminate();
  }
  std::lock_guard<std::mutex> lock_guard{mutex_};
  spans_.emplace_back(std::move(request.spans()[0]));
}
}  // namespace lightstep
