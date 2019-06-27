#pragma once

#include <atomic>
#include <memory>

#include "common/atomic_unique_ptr.h"
#include "common/circular_buffer_range.h"

namespace lightstep {
template <class T>
class CircularBuffer2 {
 public:
  explicit CircularBuffer2(size_t max_size) noexcept
      : data_{new AtomicUniquePtr<T>[max_size + 1]}, capacity_{max_size + 1} {}

  CircularBufferRange<const AtomicUniquePtr<T>> Peek() const noexcept {
    return const_cast<CircularBuffer2*>(this)->PeekImpl();
  }

  template <class Callback>
  void Consume(size_t n, Callback callback) noexcept {
    assert(n <= ComputeSize(head_, tail_));
    auto range = PeekImpl().Take(n);
    static_assert(noexcept(callback(range)), "callback not allowed to throw");
    tail_ = (tail_ + n) % static_cast<int>(capacity_);
    callback(range);
  }

  bool Add(std::unique_ptr<T>& ptr) noexcept {
    while (true) {
      int head = head_;
      int tail = tail_;

      // The circular buffer is full, so return false.
      if ((head + 1) % static_cast<int>(capacity_) == tail) {
        return false;
      }

      if (data_[head].SwapIfNull(ptr)) {
        auto new_head = (head + 1) % capacity_;
        auto expected_head = head;
        if (head_.compare_exchange_weak(expected_head, new_head,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
          // free the swapped out value
          ptr.reset();

          return true;
        }

        // If we reached this point (unlikely), it means that between the last
        // iteration elements were added and then consumed from the circular
        // buffer, so we undo the swap and attempt to add again.
        data_[head].Swap(ptr);
      }
    }
    return true;
  }

  size_t max_size() const noexcept { return capacity_ - 1; }

 private:
   std::unique_ptr<AtomicUniquePtr<T>[]> data_;
   size_t capacity_;
   std::atomic<int> head_{0};
   std::atomic<int> tail_{0};

  CircularBufferRange<AtomicUniquePtr<T>> PeekImpl() noexcept {
    int tail = tail_;
    int head = head_;
    if (head == tail) {
      return {};
    }
    auto data = data_.get();
    if (tail < head) {
      return {data + tail, data + head, nullptr, nullptr};
    }
    return {data + tail, data + capacity_, data, data + head};
  }

  size_t ComputeSize(int head, int tail) const noexcept {
    if (tail <= head) {
      return head - tail;
    }
    return capacity_ - (tail - head);
  }
};
} // namespace lightstep
