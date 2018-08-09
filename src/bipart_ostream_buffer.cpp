#include "bipart_ostream_buffer.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
BipartOstreamBuffer::BipartOstreamBuffer(char* data1, size_t size1, char* data2,
                                         size_t size2) noexcept
    : data1_{data1}, size1_{size1}, data2_{data2}, size2_{size2} {}

//------------------------------------------------------------------------------
// Next
//------------------------------------------------------------------------------
bool BipartOstreamBuffer::Next(void** data, int* size) {
  if (num_bytes_written_ < size1_) {
    *data = static_cast<void*>(data1_ + num_bytes_written_);
    *size = static_cast<int>(size1_ - num_bytes_written_);
    num_bytes_written_ += *size;
    return true;
  }
  auto num_bytes_written2 = num_bytes_written_ - size1_;
  if (num_bytes_written2 < size2_) {
    *data = static_cast<void*>(data2_ + num_bytes_written2);
    *size = static_cast<int>(size2_ - num_bytes_written2);
    num_bytes_written_ += *size;
    return true;
  }
  return false;
}
}  // namespace lightstep
