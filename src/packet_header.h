#pragma once

#include <cstdint>
#include <cstddef>

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
class PacketHeader {
 public:
   explicit PacketHeader(google::protobuf::io::CodedInputStream& istream) noexcept;

   explicit PacketHeader(const char* data) noexcept;

    PacketHeader(uint8_t version, uint32_t body_size) noexcept;

   void serialize(char* data) const noexcept;

   uint8_t version() const noexcept { return version_; }

   uint32_t body_size() const noexcept { return body_size_; }
 private:
   uint8_t version_;
   uint32_t body_size_;

 public:
  static const size_t size = sizeof(version_) + sizeof(body_size_);
};
} // namespace lightstep
