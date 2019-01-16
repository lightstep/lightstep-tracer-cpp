#include "bipart_memory_stream.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
BipartMemoryOutputStream::BipartMemoryOutputStream(char* data1, size_t size1,
                                                   char* data2,
                                                   size_t size2) noexcept
    : data1_{data1}, size1_{size1}, data2_{data2}, size2_{size2} {}

BipartMemoryInputStream::BipartMemoryInputStream(const char* data1,
                                                 size_t size1,
                                                 const char* data2,
                                                 size_t size2) noexcept
    : data1_{data1}, size1_{size1}, data2_{data2}, size2_{size2} {}

//------------------------------------------------------------------------------
// Next
//------------------------------------------------------------------------------
bool BipartMemoryOutputStream::Next(void** data, int* size) {
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

bool BipartMemoryInputStream::Next(const void** data, int* size) {
  if (num_bytes_read_ < size1_) {
    *data = static_cast<const void*>(data1_ + num_bytes_read_);
    *size = static_cast<int>(size1_ - num_bytes_read_);
    num_bytes_read_ += *size;
    return true;
  }
  auto num_bytes_read2 = num_bytes_read_ - size1_;
  if (num_bytes_read2 < size2_) {
    *data = static_cast<const void*>(data2_ + num_bytes_read2);
    *size = static_cast<int>(size2_ - num_bytes_read2);
    num_bytes_read_ += *size;
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// Skip
//------------------------------------------------------------------------------
bool BipartMemoryInputStream::Skip(int count) {
  if (count + num_bytes_read_ > size1_ + size2_) {
    return false;
  }
  num_bytes_read_ += count;
  return true;
}
}  // namespace lightstep
