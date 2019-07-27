#pragma once

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
class DirectCodedOutputStream {
 public:
  explicit DirectCodedOutputStream(google::protobuf::uint8* data) noexcept
      : data_{data} {}

  void WriteTag(uint32_t x) noexcept {
    data_ = google::protobuf::io::CodedOutputStream::WriteTagToArray(x, data_);
  }

  void WriteVarint32(uint32_t x) noexcept {
    data_ =
        google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(x, data_);
  }

  void WriteVarint64(uint64_t x) noexcept {
    data_ =
        google::protobuf::io::CodedOutputStream::WriteVarint64ToArray(x, data_);
  }

  void WriteRaw(const void* data, size_t size) noexcept {
    data_ = google::protobuf::io::CodedOutputStream::WriteRawToArray(data, size,
                                                                     data_);
  }

#if 0
  void WriteBigVarint64(uint64_t value) noexcept {
    while (value >= 0x80) {
      *data_ = static_cast<google::protobuf::uint8>(value | 0x80);
      value >>= 7;
      ++data_;
    }
    *data_++ = static_cast<google::protobuf::uint8>(value);
  }
#endif

  void WriteBigVarint64(uint64_t x) noexcept {
    static const uint64_t pow_2_56 = (1ull << 56);
    if (x < pow_2_56) {
      WriteVarint64(x);
      return;
    }
    for (int i = 0; i < 8; ++i) {
      *data_ = static_cast<google::protobuf::uint8>(x | 0x80);
      x >>= 7;
      ++data_;
    }
    if (x >= 0x80) {
      *data_ = static_cast<google::protobuf::uint8>(x | 0x80);
      x >>= 7;
      ++data_;
    }
    *data_++ = static_cast<google::protobuf::uint8>(x);
  }

 private:
  google::protobuf::uint8* data_;
};

inline void WriteBigVarint64(DirectCodedOutputStream& stream,
                             uint64_t x) noexcept {
  stream.WriteBigVarint64(x);
}
}  // namespace lightstep
