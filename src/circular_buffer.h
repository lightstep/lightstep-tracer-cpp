#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

namespace lightstep {
struct CircularBufferReservation {
  char* data1;
  size_t size1;
  char* data2;
  size_t size2;
};

class CircularBuffer {
 public:
  explicit CircularBuffer(size_t capacity);

  CircularBufferReservation Reserve(size_t num_bytes) noexcept;

  void Consume(size_t num_bytes) noexcept;

  const char* tail_data() const noexcept { return data_.get() + tail_; }

  ptrdiff_t tail_index() const noexcept { return tail_; }

  size_t capacity() const noexcept { return capacity_; }

  bool empty() const noexcept { return head_ == tail_; }

  size_t size() const noexcept;

 private:
  std::unique_ptr<char[]> data_;
  size_t capacity_;
  std::atomic<ptrdiff_t> head_;
  std::atomic<ptrdiff_t> tail_;
};
}  // namespace lightstep
