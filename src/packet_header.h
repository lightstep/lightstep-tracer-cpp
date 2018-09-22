#pragma once

#include <cstddef>
#include <cstdint>

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
enum class PacketType : uint8_t { Initiation, Span, Metrics };

class PacketHeader {
 public:
  PacketHeader() = default;

  explicit PacketHeader(
      google::protobuf::io::CodedInputStream& istream) noexcept;

  explicit PacketHeader(const char* data) noexcept;

  PacketHeader(PacketType type, uint32_t body_size) noexcept;

  void serialize(char* data) const noexcept;

  PacketType type() const noexcept { return type_; }

  uint32_t body_size() const noexcept { return body_size_; }

 private:
  PacketType type_{};
  uint32_t body_size_{0};

 public:
  static const size_t size = sizeof(type_) + sizeof(body_size_);
};
}  // namespace lightstep
