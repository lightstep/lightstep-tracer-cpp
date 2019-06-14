#pragma once

#include <mutex>

#include "recorder/recorder.h"

namespace lightstep {
/**
 * A Recorder with added handlers that get called on fork events.
 *
 * Note: this is meant to support forking from a single thread that owns a
 * recorder. If multiple threads are creating recorders or interacting with a
 * single tracer while one of the threads forks, the process may well end up in
 * an invalid state because mutexes might be left locked.
 */
class ForkAwareRecorder : public Recorder {
 public:
  ForkAwareRecorder() noexcept;

  ~ForkAwareRecorder() noexcept override;

  /**
   * Handler called prior to forking.
   */
  virtual void PrepareForFork() noexcept {}

  /**
   * Handler called in the parent process after a fork.
   */
  virtual void OnForkedParent() noexcept {}

  /**
   * Handler called in the child process after a fork.
   */
  virtual void OnForkedChild() noexcept {}

  /**
   * @return a vector of the active recorders.
   */
  static std::vector<const ForkAwareRecorder*> GetActiveRecordersForTesting();

 private:
  static std::mutex mutex_;
  static ForkAwareRecorder* active_recorders_;

  // Note: We use an intrusive linked list so as to avoid any lifetime issues
  // associated with global variables with destructors.
  ForkAwareRecorder* next_recorder_{nullptr};

  static void PrepareForkHandler() noexcept;

  static void ParentForkHandler() noexcept;

  static void ChildForkHandler() noexcept;

  static void SetupForkHandlers() noexcept;
};
}  // namespace lightstep
