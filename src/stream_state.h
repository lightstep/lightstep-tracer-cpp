#pragma once

#include "circular_buffer.h"

namespace lightstep {
class StreamState {
 public:
  explicit StreamState(const CircularBuffer& buffer);

  void OnWrite(size_t consumer_allotment, size_t num_written) noexcept;

  size_t bytes_til_next_packet() const noexcept {
    return bytes_til_next_packet_;
  }

 private:
  const CircularBuffer& buffer_;
  size_t position_{0};
  size_t bytes_til_next_packet_{0};

  size_t ComputeBytesTilNextPacket(size_t num_written) const noexcept;
};
}  // namespace lightstep
