#pragma once

#include <atomic>
#include <cstdint>
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
    assert(n <= static_cast<size_t>(head_ - tail_));
    auto range = PeekImpl().Take(n);
    static_assert(noexcept(callback(range)), "callback not allowed to throw");
    tail_ += n;
    callback(range);
  }

  bool Add(std::unique_ptr<T>& ptr) noexcept {
    while (true) {
      int64_t tail = tail_;
      int64_t head = head_;

      // The circular buffer is full, so return false.
      if (static_cast<size_t>(head - tail) >= capacity_ - 1) {
        return false;
      }

      int64_t head_index = head % capacity_;
      if (data_[head_index].SwapIfNull(ptr)) {
        auto new_head = head + 1;
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
        data_[head_index].Swap(ptr);
      }
    }
    return true;
  }

  size_t max_size() const noexcept { return capacity_ - 1; }

 private:
  std::unique_ptr<AtomicUniquePtr<T>[]> data_;
  size_t capacity_;
  std::atomic<int64_t> head_{0};
  std::atomic<int64_t> tail_{0};

  CircularBufferRange<AtomicUniquePtr<T>> PeekImpl() noexcept {
    int64_t tail_index = tail_ % capacity_;
    int64_t head_index = head_ % capacity_;
    if (head_index == tail_index) {
      return {};
    }
    auto data = data_.get();
    if (tail_index < head_index) {
      return {data + tail_index, data + head_index, nullptr, nullptr};
    }
    return {data + tail_index, data + capacity_, data, data + head_index};
  }
};
}  // namespace lightstep
