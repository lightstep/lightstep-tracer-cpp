#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace lightstep {
// Provides an interface like std::condition_variable but adds a terminate
// feature where all current and future waits can be cancelled.
//
// This can be used with Transporters to do waiting in such a way that doesn't
// stop the program from exiting quickly. For example, if a connection to a
// satellite can't be established, a transporter can sleep before retrying, but
// interrupte the sleep if the program exits.
class TerminableConditionVariable {
 public:
  TerminableConditionVariable() = default;

  void NotifyAll() noexcept { condition_variable_.notify_all(); }

  void Terminate() noexcept;

  bool is_active() const noexcept { return is_active_; }

  template <class Rep, class Period>
  bool WaitFor(const std::chrono::duration<Rep, Period>& duration) {
    std::unique_lock<std::mutex> lock{mutex_};
    return condition_variable_.wait_for(lock, duration,
                                        [this] { return !this->is_active_; });
  }

  template <class Rep, class Period, class Predicate>
  bool WaitFor(const std::chrono::duration<Rep, Period>& duration,
               Predicate predicate) {
    std::unique_lock<std::mutex> lock{mutex_};
    return condition_variable_.wait_for(lock, duration, [predicate, this] {
      return !this->is_active_ || predicate();
    });
  }

 private:
  std::mutex mutex_;

  // Why have both a mutex and an atomic for this? The atomic is for reading
  // in a loop where it's not essential to coordinate the order of actions with
  // Terminate.
  //
  // The mutex is to ensure that a thread doesn't wait on the condition
  // variable after it's been terminated.
  std::atomic<bool> is_active_{true};

  std::condition_variable condition_variable_;
};
}  // namespace lightstep
