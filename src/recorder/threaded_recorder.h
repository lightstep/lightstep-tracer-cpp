#pragma once

#include <mutex>

#include "recorder/recorder.h"

namespace lightstep {
class ThreadedRecorder : public Recorder {
 public:
  ThreadedRecorder() noexcept;

  ~ThreadedRecorder() noexcept override;

  virtual void PrepareForFork() noexcept {}

  virtual void OnForkedParent() noexcept {}

  virtual void OnForkedChild() noexcept {}

 private:
  static std::mutex mutex_;
  static ThreadedRecorder* active_recorders_;

  // Note: We use an intrusive linked list so as to avoid any lifetime issues
  // associated with global variables with destructors.
  ThreadedRecorder* next_recorder_{nullptr};

  static void PrepareForkHandler() noexcept;

  static void ParentForkHandler() noexcept;

  static void ChildForkHandler() noexcept;

  static void SetupForkHandlers() noexcept;
};
}  // namespace lightstep
