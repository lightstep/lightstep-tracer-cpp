#include "circular_buffer.h"

#include <cassert>
#include <algorithm>

namespace lightstep {
//------------------------------------------------------------------------------
// ComputeFreeSpace
//------------------------------------------------------------------------------
static size_t ComputeFreeSpace(ptrdiff_t head, ptrdiff_t tail,
                               size_t capacity) noexcept {
  if (head >= tail) {
    return capacity - 1 - (head - tail);
  } else {
    return tail - head - 1;
  }
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
CircularBuffer::CircularBuffer(size_t capacity)
    : data_{new char[capacity]}, capacity_{capacity}, head_{0}, tail_{0} {}

//------------------------------------------------------------------------------
// Reserve
//------------------------------------------------------------------------------
CircularBufferPlacement CircularBuffer::Reserve(size_t num_bytes) noexcept {
  ptrdiff_t new_head;
  ptrdiff_t head = head_;
  while (1) {
    ptrdiff_t tail = tail_;
    auto free_space = ComputeFreeSpace(head, tail, capacity_);
    if (free_space < num_bytes) {
      return {nullptr, 0, nullptr, 0};
    }

    new_head = (head + num_bytes) % capacity_;
    if (head_.compare_exchange_weak(head, new_head, std::memory_order_release,
                                    std::memory_order_relaxed)) {
      break;
    }
  }

  if (new_head > head) {
    return {data_.get() + head, num_bytes, nullptr, 0};
  }

  return {data_.get() + head, capacity_ - head, data_.get(),
          static_cast<size_t>(new_head)};
}

//------------------------------------------------------------------------------
// Peek
//------------------------------------------------------------------------------
CircularBufferConstPlacement CircularBuffer::Peek(ptrdiff_t index,
                                                  size_t num_bytes) const
    noexcept {
  assert(index + num_bytes <= size());
  ptrdiff_t tail = tail_;
  auto first_index = (tail + index) % capacity_;
  auto data1 = data_.get() + first_index;
  auto size1 = std::min(static_cast<size_t>(capacity_ - first_index), num_bytes);
  auto data2 = data_.get();
  auto size2 = num_bytes - size1;
  return {data1, size1, data2, size2};
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
void CircularBuffer::Consume(size_t num_bytes) noexcept {
  ptrdiff_t new_tail;
  ptrdiff_t tail = tail_;
  while (1) {
    assert(num_bytes <= ComputeFreeSpace(head_, tail, capacity_));
    new_tail = (tail + num_bytes) % capacity_;
    if (tail_.compare_exchange_weak(tail, new_tail, std::memory_order_release,
                                    std::memory_order_relaxed)) {
      break;
    }
  }
}

//------------------------------------------------------------------------------
// size
//------------------------------------------------------------------------------
size_t CircularBuffer::size() const noexcept {
  ptrdiff_t head = head_;
  ptrdiff_t tail = tail_;
  if (head >= tail) {
    return head - tail;
  } else {
    return capacity_ - tail - head;
  }
}
}  // namespace lightstep
