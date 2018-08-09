#pragma once

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
class BipartOstreamBuffer
    : public google::protobuf::io::ZeroCopyOutputStream {
 public:
  BipartOstreamBuffer(char* data1, size_t size1, char* data2,
                      size_t size2) noexcept;

  bool Next(void** data, int* size) final;

  void BackUp(int count) final { num_bytes_written_ -= count; }

  google::protobuf::int64 ByteCount() const final {
    return static_cast<google::protobuf::int64>(num_bytes_written_);
  }

 private:
  char *data1_;
  size_t size1_;
  char* data2_;
  size_t size2_;
  size_t num_bytes_written_ = 0;
};
} // namespace lightstep
