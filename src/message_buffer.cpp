#include "message_buffer.h"

#include "bipart_memory_stream.h"
#include "packet_header.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <array>
#include <iostream>

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
  BipartMemoryOutputStream stream_buffer{placement.data1, placement.size1,
                                         placement.data2, placement.size2};
  google::protobuf::io::CodedOutputStream stream{&stream_buffer};
  SerializePacketHeader(header, stream);
  message.SerializeWithCachedSizes(&stream);
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
MessageBuffer::MessageBuffer(size_t num_bytes,
                             MetricsObserver* metrics_observer)
    : buffer_{num_bytes},
      ready_flags_{num_bytes},
      metrics_observer_{metrics_observer} {}

//------------------------------------------------------------------------------
// Add
//------------------------------------------------------------------------------
bool MessageBuffer::Add(PacketType packet_type,
                        const google::protobuf::Message& message) noexcept {
  auto body_size = message.ByteSizeLong();

  auto placement = buffer_.Reserve(body_size + PacketHeader::size);

  // Not enough space, so drop the span.
  if (placement.data1 == nullptr) {
    if (metrics_observer_ != nullptr) {
      metrics_observer_->OnSpansDropped(1);
    }
    return false;
  }

  PacketHeader header{packet_type, static_cast<uint32_t>(body_size)};
  SerializePacket(header, message, placement);

  // Mark the span as ready to send to the satellite.
  ready_flags_.Set(static_cast<int>(placement.data1 - buffer_.data()));

  return true;
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
bool MessageBuffer::Consume(ConsumerFunction consumer, void* context) {
  GrowConsumerAllotment();
  if (consumer_allotment_ == 0) {
    return false;
  }
  auto max_num_consumption_bytes =
      std::min(consumer_allotment_, buffer_.capacity() - buffer_.tail_index());
  auto num_bytes_consumed =
      consumer(context, buffer_.tail_data(), max_num_consumption_bytes);
  buffer_.Consume(num_bytes_consumed);
  total_bytes_consumed_ += num_bytes_consumed;
  consumer_allotment_ -= num_bytes_consumed;
  return true;
}

//------------------------------------------------------------------------------
// GrowConsumerAllotment
//------------------------------------------------------------------------------
void MessageBuffer::GrowConsumerAllotment() noexcept {
  while (true) {
    auto next_packet_index =
        (buffer_.tail_index() + consumer_allotment_) % buffer_.capacity();

    // Return if the next packet isn't ready to send.
    if (!ready_flags_.Reset(static_cast<int>(next_packet_index))) {
      return;
    }

    // Deserialize the packet header so that we can determine how big it is.
    auto placement = buffer_.Peek(consumer_allotment_, PacketHeader::size);
    BipartMemoryInputStream stream_buffer{placement.data1, placement.size1,
                                          placement.data2, placement.size2};
    google::protobuf::io::CodedInputStream stream{&stream_buffer};
    std::array<char, PacketHeader::size> buffer;
    stream.ReadRaw(static_cast<void*>(buffer.data()),
                   static_cast<int>(PacketHeader::size));
    PacketHeader header{buffer.data()};

    // Add the packet to the consumer allotment
    if (metrics_observer_ != nullptr && header.type() == PacketType::Span) {
      metrics_observer_->OnSpansSent(1);
    }
    consumer_allotment_ += PacketHeader::size + header.body_size();
  }
}
}  // namespace lightstep
