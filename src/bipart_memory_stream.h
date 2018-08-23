#pragma once

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
// A ZeroCopyOutputStream that can be used to write to two separate blocks of
// memory.
//
// Note: this can be used with a circular buffer to write directly into the
// buffer if it wraps at the ends.
class BipartMemoryOutputStream
    : public google::protobuf::io::ZeroCopyOutputStream {
 public:
  BipartMemoryOutputStream(char* data1, size_t size1, char* data2,
                           size_t size2) noexcept;

  bool Next(void** data, int* size) final;

  void BackUp(int count) final { num_bytes_written_ -= count; }

  google::protobuf::int64 ByteCount() const final {
    return static_cast<google::protobuf::int64>(num_bytes_written_);
  }

 private:
  char* data1_;
  size_t size1_;
  char* data2_;
  size_t size2_;
  size_t num_bytes_written_ = 0;
};

// A ZeroCopyOutputStream that can be used to read from two separate blocks of
// memory.
//
// See BipartMemoryOutputStream
class BipartMemoryInputStream
    : public google::protobuf::io::ZeroCopyInputStream {
 public:
  BipartMemoryInputStream(const char* data1, size_t size1, const char* data2,
                          size_t size2) noexcept;

  bool Next(const void** data, int* size) final;

  void BackUp(int count) final { num_bytes_read_ -= count; }

  google::protobuf::int64 ByteCount() const final {
    return static_cast<google::protobuf::int64>(num_bytes_read_);
  }

  bool Skip(int count) final;

 private:
  const char* data1_;
  size_t size1_;
  const char* data2_;
  size_t size2_;
  size_t num_bytes_read_ = 0;
};
}  // namespace lightstep
