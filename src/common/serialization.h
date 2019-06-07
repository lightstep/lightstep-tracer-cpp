#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>

namespace lightstep {
// See https://developers.google.com/protocol-buffers/docs/encoding#structure
// Only lists the ones we use
enum class WireType {
  Varint = 0,
  LengthDelimited = 2
};

template <size_t FieldNumber, WireType WireTypeValue>
struct StaticSerializationKey {
  // See https://developers.google.com/protocol-buffers/docs/encoding#structure
  // for documentation on encoding.
  static const uint32_t value = static_cast<uint32_t>(
      (FieldNumber << 3) | static_cast<size_t>(WireTypeValue));
};

template <size_t FieldNumber>
void SerializeKeyLength(google::protobuf::io::CodedOutputStream& stream,
                        size_t length) {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::LengthDelimited>::value);
  stream.WriteVarint64(length);
}

template <size_t FieldNumber>
void SerializeVarint(google::protobuf::io::CodedOutputStream& ostream,
                     uint64_t x) {
  ostream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  ostream.WriteVarint64(x);
}

template <size_t FieldNumber>
void SerializeString(google::protobuf::io::CodedOutputStream& stream,
                     opentracing::string_view s) {
  SerializeKeyLength<FieldNumber>(stream, s.size());
  stream.WriteRaw(static_cast<const void*>(s.data()), s.size());
}
} // namespace lightstep
