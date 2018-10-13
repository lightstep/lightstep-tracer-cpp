#include "../src/stream_state.h"
#include "../src/bipart_memory_stream.h"
#include "../src/packet_header.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;

static void AddPacket(CircularBuffer& buffer, PacketType type, size_t size) {
  PacketHeader header{type, static_cast<uint32_t>(size - PacketHeader::size)};
  auto placement = buffer.Reserve(size);
  BipartMemoryOutputStream stream_buffer{placement.data1, placement.size1,
                                         placement.data2, placement.size2};
  google::protobuf::io::CodedOutputStream stream{&stream_buffer};
  WritePacketHeader(header, stream);
}

TEST_CASE("StreamState") {
  CircularBuffer buffer{100};
  AddPacket(buffer, PacketType::Span, 10);
  StreamState stream_state{buffer};

  SECTION("Upon construction there's no bytes til the next packets.") {
    CHECK(stream_state.bytes_til_next_packet() == 0);
  }

  SECTION(
      "After reading part-way into a packet, bytes_til_next_packet returns the "
      "remaining portion of the packet.") {
    stream_state.OnWrite(buffer.size(), 7);
    CHECK(stream_state.bytes_til_next_packet() == 3);
  }

  SECTION("After reading a full packet, bytes_til_next_packet is zero.") {
    stream_state.OnWrite(buffer.size(), 10);
    CHECK(stream_state.bytes_til_next_packet() == 0);
  }

  SECTION(
      "Reading part-way into a packet portion returns the remaining portion.") {
    stream_state.OnWrite(buffer.size(), 6);
    buffer.Consume(6);
    stream_state.OnWrite(buffer.size(), 3);
    CHECK(stream_state.bytes_til_next_packet() == 1);
  }

  SECTION(
      "Reading part-way into the next packet gives the correct "
      "bytes_til_next_pacet.") {
    AddPacket(buffer, PacketType::Span, 15);
    stream_state.OnWrite(buffer.size(), 13);
    CHECK(stream_state.bytes_til_next_packet() == 12);
  }

  SECTION(
      "Reading part-way into the next after packet gives the correct "
      "bytes_til_next_packet") {
    AddPacket(buffer, PacketType::Span, 15);
    AddPacket(buffer, PacketType::Span, 20);
    stream_state.OnWrite(buffer.size(), 26);
    CHECK(stream_state.bytes_til_next_packet() == 19);
  }
}
