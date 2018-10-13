#pragma once

#include <cstddef>
#include <cstdint>

#include "circular_buffer.h"

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
enum class PacketType : uint8_t { Initiation = 0, Span = 1, Metrics = 2 };

// The header for a message packet used for streaming data to a satellite.
class PacketHeader {
 public:
  PacketHeader() = default;

  explicit PacketHeader(
      google::protobuf::io::CodedInputStream& istream) noexcept;

  explicit PacketHeader(const char* data) noexcept;

  PacketHeader(PacketType type, uint32_t body_size) noexcept;

  PacketHeader(uint8_t version, PacketType type, uint32_t body_size);

  void serialize(char* data) const noexcept;

  uint8_t version() const noexcept { return version_; }

  PacketType type() const noexcept { return type_; }

  uint32_t body_size() const noexcept { return body_size_; }

  size_t packet_size() const noexcept {
    return body_size_ + PacketHeader::size;
  }

 private:
  uint8_t version_{1};
  PacketType type_{};
  uint32_t body_size_{0};

 public:
  static const size_t size =
      sizeof(version_) + sizeof(type_) + sizeof(body_size_);
};

PacketHeader ReadPacketHeader(const CircularBuffer& buffer, ptrdiff_t index);

void WritePacketHeader(
    const PacketHeader& header,
    google::protobuf::io::CodedOutputStream& stream) noexcept;
}  // namespace lightstep
