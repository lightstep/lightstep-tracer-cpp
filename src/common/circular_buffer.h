#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "common/atomic_unique_ptr.h"
#include "common/circular_buffer_range.h"

namespace lightstep {
/*
 * A lock-free circular buffer that supports multiple concurrent producers
 * and a single consumer.
 */
template <class T>
class CircularBuffer {
 public:
  explicit CircularBuffer(size_t max_size) noexcept
      : data_{new AtomicUniquePtr<T>[max_size + 1]}, capacity_{max_size + 1} {}

  /**
   * @return a range of the elements in the circular buffer
   *
   * Note: This method must only be called from the consumer thread.
   */
  CircularBufferRange<const AtomicUniquePtr<T>> Peek() const noexcept {
    return const_cast<CircularBuffer*>(this)->PeekImpl();
  }

  /**
   * Consume elements from the circular buffer's tail.
   * @param n the number of elements to consume
   * @param callback the callback to invoke with a AtomicUniquePtr to each
   * consumed element.
   *
   * Note: The callback must set the passed AtomicUniquePtr to null.
   *
   * Note: This method must only be called from the consumer thread.
   */
  template <class Callback>
  void Consume(size_t n, Callback callback) noexcept {
    assert(n <= static_cast<size_t>(head_ - tail_));
    auto range = PeekImpl().Take(n);
    static_assert(noexcept(callback(range)), "callback not allowed to throw");
    tail_ += n;
    callback(range);
  }

  /**
   * Consume elements from the circular buffer's tail.
   * @param n the number of elements to consume
   *
   * Note: This method must only be called from the consumer thread.
   */
  void Consume(size_t n) noexcept {
    Consume(
        n, [](CircularBufferRange<AtomicUniquePtr<T>> & range) noexcept {
          range.ForEach([](AtomicUniquePtr<T> & ptr) noexcept {
            ptr.Reset();
            return true;
          });
        });
  }

  /**
   * Adds an element into the circular buffer.
   * @param ptr a pointer to the element to add
   * @return true if the element was successfully added; false, otherwise.
   */
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

  /**
   * Clear the circular buffer.
   *
   * Note: This method must only be called from the consumer thread.
   */
  void Clear() noexcept { Consume(size()); }

  /**
   * @return the maximum number of bytes that can be stored in the buffer.
   */
  size_t max_size() const noexcept { return capacity_ - 1; }

  /**
   * @return true if the buffer is empty.
   */
  bool empty() const noexcept { return head_ == tail_; }

  /**
   * @return the number of bytes stored in the circular buffer.
   *
   * Note: this method will only return a correct snapshot of the size if called
   * from the consumer thread.
   */
  size_t size() const noexcept {
    uint64_t tail = tail_;
    uint64_t head = head_;
    assert(tail <= head);
    return head - tail;
  }

  /**
   * @return the number of elements consumed from the circular buffer.
   */
  int64_t consumption_count() const noexcept { return tail_; }

  /**
   * @return the number of elements added to the circular buffer.
   */
  int64_t production_count() const noexcept { return head_; }

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
