#pragma once

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
/**
 * Provides a class similar to a CodedOutputStream but that serializes directly
 * into a memory buffer that's already suitably sized.
 */
class DirectCodedOutputStream {
 public:
  explicit DirectCodedOutputStream(google::protobuf::uint8* data) noexcept
      : data_{data} {}

  /**
   * @return the attached data pointer
   */
  const google::protobuf::uint8* data() const noexcept { return data_; }

  /**
   * Profiling shows that serializing random ids is costly (which are
   * unfortunately coded as varints).
   *
   * CodedOutputStream optimizes for writing smaller integers and since random
   * 64-bit numbers are most likely larger, it's a bad fit.
   *
   * This provides alternative serialization code that's faster for large
   * integers.
   *
   * @param x the number to serialize
   */
  void WriteBigVarint64(uint64_t x) noexcept {
    static const uint64_t pow_2_56 = (1ull << 56);
    if (x < pow_2_56) {
      WriteVarint64(x);
      return;
    }
    // Make sure we get an unrolled loop. There are pragmas that would make this
    // easier, but this ensures there won't be any portability issues.
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

  // Mirrors CodedOutputStream methods.
  // See
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.coded_stream#CodedOutputStream
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

 private:
  google::protobuf::uint8* data_;
};

inline void WriteBigVarint64(DirectCodedOutputStream& stream,
                             uint64_t x) noexcept {
  stream.WriteBigVarint64(x);
}
}  // namespace lightstep
