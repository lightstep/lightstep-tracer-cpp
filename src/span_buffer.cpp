#include "span_buffer.h"

#include "packet_header.h"
#include "bipart_ostream_buffer.h"
#include <array>

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
//------------------------------------------------------------------------------
// SerializePacketHeader
//------------------------------------------------------------------------------
static void SerializePacketHeader(
    const PacketHeader& header,
    google::protobuf::io::CodedOutputStream& stream) noexcept {
  std::array<char, PacketHeader::size> serialization;
  header.serialize(serialization.data());
  stream.WriteRaw(serialization.data(), PacketHeader::size);
}

//------------------------------------------------------------------------------
// SerializePacket
//------------------------------------------------------------------------------
static void SerializePacket(const PacketHeader& header,
                            const google::protobuf::Message& message,
                            const CircularBufferPlacement& placement) noexcept {
  BipartOstreamBuffer stream_buffer{placement.data1, placement.size1,
                                    placement.data2, placement.size2};
  google::protobuf::io::CodedOutputStream stream{&stream_buffer};
  SerializePacketHeader(header, stream);
  message.SerializeWithCachedSizes(&stream);
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SpanBuffer::SpanBuffer(size_t size) : buffer_{size}, ready_flags_{size} {}

//------------------------------------------------------------------------------
// Add
//------------------------------------------------------------------------------
bool SpanBuffer::Add(const google::protobuf::Message& message) noexcept {
  auto body_size = message.ByteSizeLong();

  auto placement = buffer_.Reserve(body_size + PacketHeader::size);

  // Not enough space, so drop the span.
  if (placement.data1 == nullptr) {
    return false;
  }

  PacketHeader header{1, static_cast<uint32_t>(body_size)};
  SerializePacket(header, message, placement);

  // Mark the span as ready to send to the satellite.
  ready_flags_.Set(static_cast<int>(placement.data1 - buffer_.data()));

  return true;
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
void SpanBuffer::Consume(ConsumerFunction consumer, void* context) noexcept {
  
}

//------------------------------------------------------------------------------
// GrowConsumerAllotment
//------------------------------------------------------------------------------
void SpanBuffer::GrowConsumerAllotment() noexcept {
  while (1) {
    auto next_packet_index =
        (buffer_.tail_index() + consumer_allotment_) % buffer_.capacity();

    // Return if the next packet isn't ready to send.
    if (!ready_flags_.Reset(static_cast<int>(next_packet_index))) {
      return;
    }

    // Deserialize the packet header so that we can determine how big it is.
    /* auto head_placement = buffer_.Peek */
  }
}
}  // namespace lightstep
