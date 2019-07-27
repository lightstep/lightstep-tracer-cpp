#include "common/direct_coded_output_stream.h"

namespace lightstep {

static void WriteBigvarint64Part1(google::protobuf::uint8* data,
                                  uint64_t x) noexcept
    __attribute__((noinline));

void WriteBigvarint64Part1(google::protobuf::uint8* data, uint64_t x) noexcept {
    uint64_t temp;
    auto temp_ptr = reinterpret_cast<google::protobuf::uint8*>(&temp);
    for (int i = 0; i < 8; ++i) {
      temp_ptr[i] = static_cast<google::protobuf::uint8>((x >> 7 * i) | 0x80);
    }
    for (int i = 0; i < 8; ++i) {
      data[i] = temp_ptr[i];
    }
}

//--------------------------------------------------------------------------------------------------
// WriteBigVarint64
//--------------------------------------------------------------------------------------------------
void DirectCodedOutputStream::WriteBigVarint64(uint64_t x) noexcept {
    static const uint64_t pow_2_56 = (1ull << 56);
    if (x < pow_2_56) {
      WriteVarint64(x);
      return;
    }
    WriteBigvarint64Part1(data_, x);
    x >>= 56;
    data_ += 8;
    if (x >= 0x80) {
      *data_ = static_cast<google::protobuf::uint8>(x | 0x80);
      x >>= 7;
      ++data_;
    }
    *data_++ = static_cast<google::protobuf::uint8>(x);
}
} // namespace lightstep
