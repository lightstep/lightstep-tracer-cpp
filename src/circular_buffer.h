#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

namespace lightstep {
struct CircularBufferPlacement {
  char* data1;
  size_t size1;
  char* data2;
  size_t size2;
};

struct CircularBufferConstPlacement {
  const char* data1;
  size_t size1;
  const char* data2;
  size_t size2;
};

class CircularBuffer {
 public:
  explicit CircularBuffer(size_t capacity);

  CircularBufferPlacement Reserve(size_t num_bytes) noexcept;

  CircularBufferConstPlacement Peek(ptrdiff_t index, size_t num_bytes) const
      noexcept;

  void Consume(size_t num_bytes) noexcept;

  const char* data() const noexcept { return data_.get(); }

  char* data() noexcept { return data_.get(); }

  const char* tail_data() const noexcept { return data_.get() + tail_; }
  char* tail_data() noexcept { return data_.get() + tail_; }
  ptrdiff_t tail_index() const noexcept { return tail_; }

  const char* head_data() const noexcept { return data_.get() + head_; }
  char* head_data() noexcept { return data_.get() + head_; }
  ptrdiff_t head_index() const noexcept { return head_; }

  size_t capacity() const noexcept { return capacity_; }

  size_t free_space() const noexcept { return capacity_ - 1 - size(); }

  bool empty() const noexcept { return head_ == tail_; }

  size_t size() const noexcept;

 private:
  std::unique_ptr<char[]> data_;
  size_t capacity_;
  std::atomic<ptrdiff_t> head_;
  std::atomic<ptrdiff_t> tail_;
};
}  // namespace lightstep
