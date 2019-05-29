#include "recorder/threaded_recorder.h"

#include <cassert>

#include <pthread.h>

namespace lightstep {
std::mutex ThreadedRecorder::mutex_;

ThreadedRecorder* ThreadedRecorder::active_recorders_{nullptr};

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ThreadedRecorder::ThreadedRecorder() noexcept {
  SetupForkHandlers();
  std::lock_guard<std::mutex> lock_guard{mutex_};
  next_recorder_ = active_recorders_;
  active_recorders_ = this;
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
ThreadedRecorder::~ThreadedRecorder() noexcept {
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
  assert(0 && "ThreadedRecorder not found in global list");
}

//--------------------------------------------------------------------------------------------------
// PrepareForkHandler
//--------------------------------------------------------------------------------------------------
void ThreadedRecorder::PrepareForkHandler() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    recorder->PrepareForFork();
  }
}

//--------------------------------------------------------------------------------------------------
// ParentForkHandler
//--------------------------------------------------------------------------------------------------
void ThreadedRecorder::ParentForkHandler() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    recorder->OnForkedParent();
  }
}

//--------------------------------------------------------------------------------------------------
// ChildForkHandler
//--------------------------------------------------------------------------------------------------
void ThreadedRecorder::ChildForkHandler() noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto recorder = active_recorders_; recorder != nullptr;
       recorder = recorder->next_recorder_) {
    recorder->OnForkedChild();
  }
}

//--------------------------------------------------------------------------------------------------
// SetupForkHandlers
//--------------------------------------------------------------------------------------------------
void ThreadedRecorder::SetupForkHandlers() noexcept {
  static bool once = [] {
    ::pthread_atfork(PrepareForkHandler, ParentForkHandler, ChildForkHandler);
    return true;
  }();
  (void)once;
}
}  // namespace lightstep
