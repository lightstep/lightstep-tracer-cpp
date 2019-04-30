#include "common/circular_buffer.h"

#include <algorithm>
#include <cassert>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
CircularBuffer::CircularBuffer(size_t max_size)
    : data_{new char[max_size + 1]}, capacity_{max_size + 1} {}

//------------------------------------------------------------------------------
// Reserve
//------------------------------------------------------------------------------
CircularBufferPlacement CircularBuffer::Reserve(size_t num_bytes) noexcept {
  uint64_t new_head;
  uint64_t head = head_;
  while (true) {
    uint64_t tail = tail_;

    // Because we update head_ before tail_, it's possible that tail > head; if
    // that's the case, then the compare_exchange_weak check below will fail and
    // we'll reloop, but we use std::min here to ensure that we don't get some
    // nonsensical result for the free_space because of overflow.
    auto free_space =
        max_size() - static_cast<size_t>((head - std::min(head, tail)));
    if (free_space < num_bytes) {
      return {nullptr, 0, nullptr, 0};
    }

    new_head = head + num_bytes;
    if (head_.compare_exchange_weak(head, new_head, std::memory_order_release,
                                    std::memory_order_relaxed)) {
      break;
    }
  }
  return getPlacement(head, num_bytes);
}

//------------------------------------------------------------------------------
// Peek
//------------------------------------------------------------------------------
CircularBufferConstPlacement CircularBuffer::Peek(size_t index,
                                                  size_t num_bytes) const
    noexcept {
  assert(index + num_bytes <= size());
  return getPlacement(tail_ + index, num_bytes);
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
void CircularBuffer::Consume(size_t num_bytes) noexcept {
  assert(tail_ + num_bytes <= head_);
  tail_ += num_bytes;
}

//--------------------------------------------------------------------------------------------------
// ComputePosition
//--------------------------------------------------------------------------------------------------
size_t CircularBuffer::ComputePosition(const char* ptr) const noexcept {
  auto index = static_cast<uint64_t>(ptr - this->data());
  uint64_t tail = tail_;
  tail = tail % capacity_;
  if (index >= tail) {
    return index - tail;
  }
  return capacity_ - tail + index;
}

//--------------------------------------------------------------------------------------------------
// size
//--------------------------------------------------------------------------------------------------
size_t CircularBuffer::size() const noexcept {
  uint64_t tail = tail_;
  uint64_t head = head_;
  assert(tail <= head);
  return head - tail;
}

//--------------------------------------------------------------------------------------------------
// getPlacement
//--------------------------------------------------------------------------------------------------
CircularBufferPlacement CircularBuffer::getPlacement(
    size_t index, size_t num_bytes) noexcept {
  index = index % capacity_;
  auto data1 = data_.get() + index;
  auto size1 = std::min(capacity_ - index, num_bytes);
  auto data2 = data_.get();
  auto size2 = num_bytes - size1;
  return {data1, size1, data2, size2};
}
}  // namespace lightstep
