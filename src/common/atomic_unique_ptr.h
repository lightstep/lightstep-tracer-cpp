#pragma once

#include <atomic>
#include <memory>

namespace lightstep {
template <class T>
class AtomicUniquePtr {
 public:
  AtomicUniquePtr() noexcept = default;

  ~AtomicUniquePtr() noexcept { Reset(); }

  bool IsNull() const noexcept { return ptr_ == nullptr; }

  bool SwapIfNull(std::unique_ptr<T>& owner) noexcept {
    auto ptr = owner.get();
    T* expected = nullptr;
    auto was_successful = ptr_.compare_exchange_weak(
        expected, ptr, std::memory_order_release, std::memory_order_relaxed);
    if (was_successful) {
      owner.release();
      return true;
    }
    return false;
  }

  void Swap(std::unique_ptr<T>& ptr) noexcept {
    ptr.reset(ptr_.exchange(ptr.release()));
  }

  void Reset(T* ptr = nullptr) noexcept {
    ptr = ptr_.exchange(ptr);
    if (ptr != nullptr) {
      delete ptr;
    }
  }

  T* Get() noexcept { return ptr_; }

  const T* Get() const noexcept { return ptr_; }

 private:
  std::atomic<T*> ptr_{nullptr};
};
}  // namespace lightstep
