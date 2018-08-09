#include "span_buffer.h"

#include "packet_header.h"
#include "bipart_ostream_buffer.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
//------------------------------------------------------------------------------
// SerializePacket
//------------------------------------------------------------------------------
static void SerializePacket(const PacketHeader& header,
                            const google::protobuf::Message& message,
                            const CircularBufferPlacement& placement) noexcept {
  
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SpanBuffer::SpanBuffer(size_t size) : buffer_{size}, ready_flags_{size} {}

//------------------------------------------------------------------------------
// Add
//------------------------------------------------------------------------------
bool SpanBuffer::Add(const google::protobuf::Message& message) noexcept {
  auto serialization_size = message.ByteSizeLong();

  auto placement = buffer_.Reserve(serialization_size + PacketHeader::size);

  // Not enough space, so drop the span
  if (placement.data1 == nullptr) {
    return false;
  }

  return true;
}
}  // namespace lightstep
