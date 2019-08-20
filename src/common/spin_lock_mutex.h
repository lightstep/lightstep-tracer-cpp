#pragma once

#include <atomic>
#include <mutex>

namespace lightstep {
/**
 * Simple spin lock implementation.
 *
 * See https://stackoverflow.com/a/29195378/4447365
 */
class SpinLockMutex {
 public:
  // std Mutex requirements
  inline void lock() noexcept {
    while (locked_.test_and_set(std::memory_order_acquire)) {
    }
  }

  inline void unlock() noexcept { locked_.clear(std::memory_order_release); }

 private:
  std::atomic_flag locked_ = ATOMIC_FLAG_INIT;
};

using SpinLockGuard = std::lock_guard<SpinLockMutex>;
}  // namespace lightstep
