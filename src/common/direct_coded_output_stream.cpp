#include "common/direct_coded_output_stream.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// WriteBigVarint64
//--------------------------------------------------------------------------------------------------
void DirectCodedOutputStream::WriteBigVarint64(uint64_t x) noexcept {
  static const uint64_t pow_2_56 = (1ull << 56);
  if (x < pow_2_56) {
    WriteVarint64(x);
    return;
  }
#define DO_ITERATION                                           \
  {                                                            \
    *data_++ = static_cast<google::protobuf::uint8>(x | 0x80); \
    x >>= 7;                                                   \
  }
  DO_ITERATION;  // 1
  DO_ITERATION;  // 2
  DO_ITERATION;  // 3
  DO_ITERATION;  // 4
  DO_ITERATION;  // 5
  DO_ITERATION;  // 6
  DO_ITERATION;  // 7
  DO_ITERATION;  // 8
  if (x >= 0x80) {
    DO_ITERATION;
  }
  *data_++ = static_cast<google::protobuf::uint8>(x);
#undef DO_ITERATION
}
} // namespace lightstep
