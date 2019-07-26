#include "recorder/fork_aware_recorder.h"

#include <cassert>

#include "common/platform/fork.h"

namespace lightstep {
std::mutex ForkAwareRecorder::mutex_;

ForkAwareRecorder* ForkAwareRecorder::active_recorders_{nullptr};

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ForkAwareRecorder::ForkAwareRecorder() noexcept {
  SetupForkHandlers();
  std::lock_guard<std::mutex> lock_guard{mutex_};
  next_recorder_ = active_recorders_;
  active_recorders_ = this;
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
ForkAwareRecorder::~ForkAwareRecorder() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  if (this == active_recorders_) {
    active_recorders_ = active_recorders_->next_recorder_;
    return;
  }
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    if (recorder->next_recorder_ == this) {
      recorder->next_recorder_ = this->next_recorder_;
      return;
    }
  }
  assert(0 && "ForkAwareRecorder not found in global list");
}

//--------------------------------------------------------------------------------------------------
// GetActiveRecordersForTesting
//--------------------------------------------------------------------------------------------------
std::vector<const ForkAwareRecorder*>
ForkAwareRecorder::GetActiveRecordersForTesting() {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  std::vector<const ForkAwareRecorder*> result;
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    result.push_back(recorder);
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// PrepareForkHandler
//--------------------------------------------------------------------------------------------------
void ForkAwareRecorder::PrepareForkHandler() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    recorder->PrepareForFork();
  }
}

//--------------------------------------------------------------------------------------------------
// ParentForkHandler
//--------------------------------------------------------------------------------------------------
void ForkAwareRecorder::ParentForkHandler() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    recorder->OnForkedParent();
  }
}

//--------------------------------------------------------------------------------------------------
// ChildForkHandler
//--------------------------------------------------------------------------------------------------
void ForkAwareRecorder::ChildForkHandler() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    recorder->OnForkedChild();
  }
}

//--------------------------------------------------------------------------------------------------
// SetupForkHandlers
//--------------------------------------------------------------------------------------------------
void ForkAwareRecorder::SetupForkHandlers() noexcept {
  static bool once = [] {
    AtFork(PrepareForkHandler, ParentForkHandler, ChildForkHandler);
    return true;
  }();
  (void)once;
}
}  // namespace lightstep
