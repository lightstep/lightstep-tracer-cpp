#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>

#include "common/utility.h"

namespace lightstep {
// See https://developers.google.com/protocol-buffers/docs/encoding#structure
// Only lists the ones we use
enum class WireType {
  Varint = 0,
  Fixed64 = 1,
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
size_t ComputeLengthDelimitedSerializationSize(size_t length) {
  return StaticKeySerializationSize<FieldNumber,
                                    WireType::LengthDelimited>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(length) + length;
}

template <size_t FieldNumber>
void SerializeKeyLength(google::protobuf::io::CodedOutputStream& stream,
                        size_t length) noexcept {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::LengthDelimited>::value);
  stream.WriteVarint64(length);
}

template <size_t FieldNumber>
void SerializeFixed64(google::protobuf::io::CodedOutputStream& stream,
                      const void* data) noexcept {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Fixed64>::value);
  stream.WriteRaw(data, sizeof(int64_t));
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
  // See https://github.com/protocolbuffers/protobuf/blob/8489612dadd3775ffbba029a583b6f00e91d0547/src/google/protobuf/timestamp.proto
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

size_t ComputeKeyValueSerializationSize(opentracing::string_view key,
                                 const opentracing::Value& value,
                                 std::string& json, int& json_counter);

void SerializeKeyValueImpl(google::protobuf::io::CodedOutputStream& stream,
                           opentracing::string_view key,
                           const opentracing::Value& value,
                           const std::string& json, int& json_counter);

template <size_t FieldNumber>
void SerializeKeyValue(google::protobuf::io::CodedOutputStream& stream,
                       opentracing::string_view key,
                       const opentracing::Value& value,
                       size_t serialization_size, const std::string& json,
                       int& json_counter) {
  SerializeKeyLength<FieldNumber>(stream, serialization_size);
  SerializeKeyValueImpl(stream, key, value, json, json_counter);
}

template <size_t FieldNumber>
void SerializeKeyValue(google::protobuf::io::CodedOutputStream& stream,
                       opentracing::string_view key,
                       const opentracing::Value& value) {
  std::string json;
  int json_counter = 0;
  auto serialization_size =
      ComputeKeyValueSerializationSize(key, value, json, json_counter);
  SerializeKeyValue<FieldNumber>(stream, key, value, serialization_size, json,
                                 json_counter);
}
} // namespace lightstep
