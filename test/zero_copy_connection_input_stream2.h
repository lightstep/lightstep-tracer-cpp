#pragma once

#include "recorder/stream_recorder/connection_stream2.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
/**
 * Wraps a ConnectionStream to expose a ZeroCopyInputStream interface.
 *
 * Reads bytes from the ConnectionStream in random amounts so as to simulate
 * different scenarios for testing.
 */
class ZeroCopyConnectionInputStream
    : public google::protobuf::io::ZeroCopyInputStream {
 public:
  explicit ZeroCopyConnectionInputStream(ConnectionStream2& stream);

  // google::protobuf::io::ZeroCopyInputStream
  bool Next(const void** data, int* size) override;

  void BackUp(int count) override;

  bool Skip(int count) override;

  google::protobuf::int64 ByteCount() const override { return byte_count_; }

 private:
  ConnectionStream2& stream_;
  std::string buffer_;
  google::protobuf::int64 byte_count_{0};
  int position_{0};

  void SetBuffer();
};
}  // namespace lightstep
