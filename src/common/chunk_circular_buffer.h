#pragma once

#include <tuple>

#include "common/atomic_bit_set.h"
#include "common/circular_buffer.h"
#include "common/function_ref.h"

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
/**
 * Maintains a lock-free circular buffer of serialized segments. Each segment is
 * framed for http/1.1 streaming as a chunk.
 *
 * See https://en.wikipedia.org/wiki/Chunked_transfer_encoding
 */
class ChunkCircularBuffer {
 public:
  using Serializer =
      FunctionRef<void(google::protobuf::io::CodedOutputStream&)>;

  explicit ChunkCircularBuffer(size_t max_bytes);

  /**
   * Serializes data into the circular buffer as a chunk.
   * @param serializer supplies a function that writes data into a
   * CodedOutputStream that writes data into the circular buffer.
   * @param size supplies the number of bytes of the serialization.
   * @return true if the the serialization was added, or false if not enough
   * space could be reserved.
   */
  bool Add(Serializer serializer, size_t size) noexcept;

  /**
   * Marks memory starting from the circular buffer's tail where serialization
   * has finished. Once the memory is alloted is safe to read.
   * @return the number of chunks allotted
   *
   * Note: Cannot be called concurrently with Consume.
   */
  int Allot() noexcept;

  /**
   * Frees serialized space in the circular buffer starting from the tail.
   * @param num_bytes supplies the number of bytes to consume.
   *
   * Note: Only bytes that have been allotted can be freed.
   */
  void Consume(size_t num_bytes) noexcept;

  /**
   * Locates the chunk in the circular buffer that contains a given pointer.
   * @param start a chunk boundary position to start searching from.
   * @param ptr the pointer to search for.
   * @return the chunk in the circular buffer that contains ptr and the number
   * of preceding chunks.
   *
   * Note: both start and ptr must be within the allotment.
   */
  std::tuple<CircularBufferConstPlacement, int> FindChunk(const char* start,
                                                          const char* ptr) const
      noexcept;

  /**
   * @return A placement referencing all data that has been allotted.
   */
  CircularBufferConstPlacement allotment() const noexcept {
    return buffer_.Peek(0, num_bytes_allotted_);
  }

  /**
   * @return the CircularBuffer where chunks are stored.
   */
  const CircularBuffer& buffer() const noexcept { return buffer_; }

  /**
   * @return the number of bytes that have been allotted.
   */
  size_t num_bytes_allotted() const noexcept { return num_bytes_allotted_; }

 private:
  AtomicBitSet ready_flags_;
  CircularBuffer buffer_;
  size_t num_bytes_allotted_{0};
};
}  // namespace lightstep
