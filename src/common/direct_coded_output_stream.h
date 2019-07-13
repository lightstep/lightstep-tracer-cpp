#pragma once

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
class DirectCodedOutputStream {
 public:
  explicit DirectCodedOutputStream(google::protobuf::uint8* data) noexcept
      : data_{data} {}

  void WriteVarint32(uint32_t x) noexcept {
    data_ =
        google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(x, data_);
   }

   void WriteVarint64(uint64_t x) noexcept {
     data_ = google::protobuf::io::CodedOutputStream::WriteVarint64ToArray(x, data_);
   }

   void WriteRaw(const void* data, size_t size) noexcept {
     data_ = google::protobuf::io::CodedOutputStream::WriteRawToArray(data, size, data_);
   }
 private:
   google::protobuf::uint8* data_;
};
} // namespace lightstep
