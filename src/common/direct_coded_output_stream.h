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

#if 0
  void WriteBigVarint64(uint64_t x) noexcept {
    static const uint64_t pow_2_56 = (1ull << 56);
    if (x < pow_2_56) {
      WriteVarint64(x);
      return;
    }

    auto iteration = [&] {
      *data_ = static_cast<google::protobuf::uint8>(x | 0x80);
      x >>= 7;
      ++data_;
    };

    /* for (int i = 0; i < 8; ++i) { */
    /*   *data_ = static_cast<google::protobuf::uint8>(x | 0x80); */
    /*   x >>= 7; */
    /*   ++data_; */
    /* } */
    iteration(); // 1
    iteration(); // 2
    iteration(); // 3
    iteration(); // 4
    iteration(); // 5
    iteration(); // 6
    iteration(); // 7
    iteration(); // 8

    if (x >= 0x80) {
      iteration();
      /* *data_ = static_cast<google::protobuf::uint8>(x | 0x80); */
      /* x >>= 7; */
      /* ++data_; */
    }
    *data_++ = static_cast<google::protobuf::uint8>(x);
  }
#endif

#if 0
  void WriteBigVarint64(uint64_t x) noexcept {
    static const uint64_t pow_2_56 = (1ull << 56);
    if (x < pow_2_56) {
      WriteVarint64(x);
      return;
    }
    uint64_t temp;
    auto temp_ptr = reinterpret_cast<google::protobuf::uint8*>(&temp);
    for (int i = 0; i < 8; ++i) {
      temp_ptr[i] = static_cast<google::protobuf::uint8>((x >> 7 * i) | 0x80);
    }
    for (int i = 0; i < 8; ++i) {
      data_[i] = temp_ptr[i];
    }
    data_ += 8;
    x >>= 56;
    if (x >= 0x80) {
      *data_ = static_cast<google::protobuf::uint8>(x | 0x80);
      x >>= 7;
      ++data_;
    }
    *data_++ = static_cast<google::protobuf::uint8>(x);
  }
#endif

  void WriteBigVarint64(uint64_t x) noexcept;

 private:
  google::protobuf::uint8* data_;
};

inline void WriteBigVarint64(DirectCodedOutputStream& stream,
                             uint64_t x) noexcept {
  stream.WriteBigVarint64(x);
}
}  // namespace lightstep
