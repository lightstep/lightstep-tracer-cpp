#pragma once

#include <cstdint>

#include "common/direct_coded_output_stream.h"

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>

namespace lightstep {
struct ValueSerializerData {
  union {
    opentracing::string_view string_view_value;
    bool bool_value;
    int64_t int64_value;
    double double_value;
  };

  ValueSerializerData() noexcept {}
};

template <class Stream>
using ValueSerializerFunction = void (*)(Stream& stream,
                                         const ValueSerializerData& data);

class ValueSerializer {
 public:
  struct json_tag {};

  ValueSerializer() noexcept = default;

  explicit ValueSerializer(opentracing::string_view value) noexcept;

  explicit ValueSerializer(json_tag, opentracing::string_view value) noexcept;

  explicit ValueSerializer(bool value) noexcept;

  explicit ValueSerializer(int64_t value) noexcept;

  explicit ValueSerializer(double value) noexcept;

  void operator()(google::protobuf::io::CodedOutputStream& stream) const {
    serializer_(stream, data_);
  }

  void operator()(DirectCodedOutputStream& stream) const noexcept {
    direct_serializer_(stream, data_);
  }

 private:
  ValueSerializerData data_;
  ValueSerializerFunction<google::protobuf::io::CodedOutputStream> serializer_;
  ValueSerializerFunction<DirectCodedOutputStream> direct_serializer_;
};
}  // namespace lightstep
