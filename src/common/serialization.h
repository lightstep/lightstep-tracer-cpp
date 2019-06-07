#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>

#include "common/utility.h"

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

template <size_t FieldNumber, WireType WireTypeValue>
struct StaticKeySerializationSize {
  static const size_t value =
      google::protobuf::io::CodedOutputStream::StaticVarintSize32<
          StaticSerializationKey<FieldNumber, WireTypeValue>::value>::value;
};

template <size_t FieldNumber>
size_t ComputeVarintSerializationSize(uint64_t x) {
  return StaticKeySerializationSize<FieldNumber, WireType::Varint>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(x);
}

template <size_t FieldNumber>
size_t ComputeVarintSerializationSize(uint32_t x) {
  return StaticKeySerializationSize<FieldNumber, WireType::Varint>::value +
         google::protobuf::io::CodedOutputStream::VarintSize32(x);
}

template <size_t FieldNumber>
void SerializeKeyLength(google::protobuf::io::CodedOutputStream& stream,
                        size_t length) noexcept {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::LengthDelimited>::value);
  stream.WriteVarint64(length);
}

template <size_t FieldNumber>
void SerializeVarint(google::protobuf::io::CodedOutputStream& stream,
                     uint64_t x) noexcept {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  stream.WriteVarint64(x);
}

template <size_t FieldNumber>
void SerializeVarint(google::protobuf::io::CodedOutputStream& stream,
                     uint32_t x) {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  stream.WriteVarint32(x);
}

template <size_t FieldNumber>
void SerializeString(google::protobuf::io::CodedOutputStream& stream,
                     opentracing::string_view s) noexcept {
  SerializeKeyLength<FieldNumber>(stream, s.size());
  stream.WriteRaw(static_cast<const void*>(s.data()), s.size());
}

template <size_t FieldNumber>
void SerializeTimestamp(google::protobuf::io::CodedOutputStream& stream,
    std::chrono::system_clock::time_point timestamp) noexcept {
  // See https://github.com/protocolbuffers/protobuf/blob/8489612dadd3775ffbba029a583b6f00e91d0547/src/google/protobuf/timestamp.pr      oto
  static const size_t SecondsSinceEpochField = 1;
  static const size_t NanoFractionField = 2;
  uint64_t seconds_since_epoch;
  uint32_t nano_fraction;
  std::tie(seconds_since_epoch, nano_fraction) =
      ProtobufFormatTimestamp(timestamp);
  auto serialization_size =
      ComputeVarintSerializationSize<SecondsSinceEpochField>(seconds_since_epoch) +
      ComputeVarintSerializationSize<NanoFractionField>(nano_fraction);
  SerializeKeyLength<FieldNumber>(stream, serialization_size);
  SerializeVarint<SecondsSinceEpochField>(stream, seconds_since_epoch);
  SerializeVarint<NanoFractionField>(stream, nano_fraction);
}
} // namespace lightstep
