#include "stream_state.h"

#include "packet_header.h"

#include <cassert>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamState::StreamState(const CircularBuffer& buffer) : buffer_{buffer} {}

//------------------------------------------------------------------------------
// OnWrite
//------------------------------------------------------------------------------
void StreamState::OnWrite(size_t consumer_allotment,
                          size_t num_written) noexcept {
  if (num_written == 0) {
    return;
  }

  if (num_written == consumer_allotment) {
    // consumer_allotment always ends at the boundary of a packet
    bytes_til_next_packet_ = 0;
  } else if (num_written <= bytes_til_next_packet_) {
    bytes_til_next_packet_ -= num_written;
  } else {
    // We need to read out packet headers starting at position_ +
    // bytes_til_next_packet, until we find out where we ended up.
    bytes_til_next_packet_ = ComputeBytesTilNextPacket(num_written);
  }

  position_ = (position_ + num_written) % buffer_.capacity();
}

//------------------------------------------------------------------------------
// ComputeBytesTilNextPacket
//------------------------------------------------------------------------------
size_t StreamState::ComputeBytesTilNextPacket(size_t num_written) const
    noexcept {
  assert(num_written > bytes_til_next_packet_);
  assert(num_written < buffer_.size());

  auto index = position_ + bytes_til_next_packet_;
  num_written -= bytes_til_next_packet_;

  while (1) {
    auto header = ReadPacketHeader(buffer_, index);
    auto packet_size = header.packet_size();
    if (packet_size >= num_written) {
      return packet_size - num_written;
    }
    num_written -= packet_size;
    index += packet_size;
  }
}
}  // namespace lightstep
