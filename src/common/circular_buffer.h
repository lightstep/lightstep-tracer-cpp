#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace lightstep {
/**
 * Represents a block of data in a circular buffer.
 *
 * If the data is contiguous in memory, data1 points to the area its area in
 * memory and size1 equals the total size. Otherwise, if the data wraps the ends
 * around the ends of the circular buffer, data1, size1 represents the first
 * block and data2, size2 represents the second block.
 */
struct CircularBufferPlacement {
  char* data1;
  size_t size1;
  char* data2;
  size_t size2;
};

/**
 * Represents a const block of data in a circular buffer.
 *
 * See CircularBufferPlacement.
 */
struct CircularBufferConstPlacement {
  CircularBufferConstPlacement() noexcept = default;

  CircularBufferConstPlacement(const char* data1_, size_t size1_,
                               const char* data2_, size_t size2_) noexcept
      : data1{data1_}, size1{size1_}, data2{data2_}, size2{size2_} {}

  CircularBufferConstPlacement(CircularBufferPlacement placement) noexcept
      : CircularBufferConstPlacement{placement.data1, placement.size1,
                                     placement.data2, placement.size2} {}

  const char* data1;
  size_t size1;
  const char* data2;
  size_t size2;
};

/*
 * A lock-free circular buffer that supports multiple concurrent producers
 * and a single consumer.
 */
class CircularBuffer {
 public:
  explicit CircularBuffer(size_t max_size);

  /**
   * Atomically reserves space in the buffer.
   * @param num_bytes supplies the number of bytes to reserve.
   * @return a CircularBufferPlacement for the reserved space if successful;
   * otherwise, a placement with data1 == nullptr if there is not enough space
   * available.
   */
  CircularBufferPlacement Reserve(size_t num_bytes) noexcept;

  /**
   * Peeks at memory stored within the buffer.
   * @param index supplies the position from the tail to take memory from.
   * @param num_bytes supplies the number of bytes to peek at.
   * @return a placement pointing to the requested memory.
   *
   * Note: This function cannot be called concurrently with Consume.
   */
  CircularBufferConstPlacement Peek(size_t index, size_t num_bytes) const
      noexcept;

  /**
   * Peeks at all memory stored in the buffer starting at a given position.
   * @param index supplies the position from the tail to take memory from.
   * @return a placement pointing to the requested memory.
   *
   * Note: This function cannot be called concurrently with Consume.
   */
  CircularBufferConstPlacement PeekFromPosition(size_t index) const noexcept {
    return Peek(index, size() - index);
  }

  /**
   * Frees reserved space in the buffer starting from the tail.
   * @param num_bytes supplies the number of bytes to consume.
   */
  void Consume(size_t num_bytes) noexcept;

  /**
   * Determine how far a pointer is from the tail of the circular buffer.
   * @param ptr the pointer to compute the position of.
   * @return the number of bytes ptr is from the tail of the circular buffer.
   */
  size_t ComputePosition(const char* ptr) const noexcept;

  /**
   * @return the maximum number of bytes that can be stored in the buffer.
   */
  size_t max_size() const noexcept { return capacity_ - 1; }

  /**
   * @return the number of bytes of free space available in the buffer.
   */
  size_t free_space() const noexcept { return max_size() - size(); }

  /**
   * @return true if the buffer is empty.
   */
  bool empty() const noexcept { return head_ == tail_; }

  /**
   * @return the number of bytes stored in the circular buffer.
   */
  size_t size() const noexcept;

  /**
   * @return a pointer to the buffer's memory.
   */
  const char* data() const noexcept { return data_.get(); }

  /**
   * @return the number of bytes consumed from the circular buffer.
   */
  uint64_t num_bytes_consumed() const noexcept { return tail_; }

  /**
   * @return the number of bytes added to the circular buffer.
   */
  uint64_t num_bytes_produced() const noexcept { return head_; }

 private:
  std::unique_ptr<char[]> data_;
  size_t capacity_;
  std::atomic<uint64_t> head_{0};
  std::atomic<uint64_t> tail_{0};

  CircularBufferPlacement getPlacement(size_t index, size_t num_bytes) noexcept;

  CircularBufferConstPlacement getPlacement(size_t index,
                                            size_t num_bytes) const noexcept {
    return const_cast<CircularBuffer*>(this)->getPlacement(index, num_bytes);
  }
};
}  // namespace lightstep
