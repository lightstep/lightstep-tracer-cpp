#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>

#include "common/utility.h"

namespace lightstep {
// See https://developers.google.com/protocol-buffers/docs/encoding#structure
// Only lists the ones we use
enum class WireType { Varint = 0, Fixed64 = 1, LengthDelimited = 2 };

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
void WriteVarint(google::protobuf::io::CodedOutputStream& stream,
                 uint64_t x) noexcept {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  stream.WriteVarint64(x);
}

template <size_t FieldNumber>
void WriteVarint(google::protobuf::io::CodedOutputStream& stream, uint32_t x) {
  stream.WriteVarint32(
      StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  stream.WriteVarint32(x);
}

template <size_t FieldNumber>
void WriteString(google::protobuf::io::CodedOutputStream& stream,
                 opentracing::string_view s) noexcept {
  SerializeKeyLength<FieldNumber>(stream, s.size());
  stream.WriteRaw(static_cast<const void*>(s.data()), s.size());
}

size_t ComputeTimestampSerializationSize(uint64_t seconds_since_epoch,
                                         uint32_t nano_fraction) noexcept;

void WriteTimestampImpl(google::protobuf::io::CodedOutputStream& stream,
                        uint64_t seconds_since_epoch,
                        uint32_t nano_fraction) noexcept;

template <size_t FieldNumber>
void WriteTimestamp(google::protobuf::io::CodedOutputStream& stream,
                    size_t serialization_size, uint64_t seconds_since_epoch,
                    uint32_t nano_fraction) noexcept {
  SerializeKeyLength<FieldNumber>(stream, serialization_size);
  WriteTimestampImpl(stream, seconds_since_epoch, nano_fraction);
}

template <size_t FieldNumber>
void WriteTimestamp(google::protobuf::io::CodedOutputStream& stream,
                    std::chrono::system_clock::time_point timestamp) noexcept {
  uint64_t seconds_since_epoch;
  uint32_t nano_fraction;
  std::tie(seconds_since_epoch, nano_fraction) =
      ProtobufFormatTimestamp(timestamp);
  auto serialization_size =
      ComputeTimestampSerializationSize(seconds_since_epoch, nano_fraction);
  WriteTimestamp<FieldNumber>(stream, serialization_size, seconds_since_epoch,
                              nano_fraction);
}

size_t ComputeKeyValueSerializationSize(opentracing::string_view key,
                                        const opentracing::Value& value,
                                        std::string& json, int& json_counter);

void WriteKeyValueImpl(google::protobuf::io::CodedOutputStream& stream,
                       opentracing::string_view key,
                       const opentracing::Value& value,
                       const std::string* json_values, int& json_counter);

template <size_t FieldNumber>
void WriteKeyValue(google::protobuf::io::CodedOutputStream& stream,
                   size_t serialization_size, opentracing::string_view key,
                   const opentracing::Value& value,
                   const std::string* json_values, int& json_counter) {
  SerializeKeyLength<FieldNumber>(stream, serialization_size);
  WriteKeyValueImpl(stream, key, value, json_values, json_counter);
}

template <size_t FieldNumber>
void WriteKeyValue(google::protobuf::io::CodedOutputStream& stream,
                   opentracing::string_view key,
                   const opentracing::Value& value) {
  std::string json;
  int json_counter = 0;
  auto serialization_size =
      ComputeKeyValueSerializationSize(key, value, json, json_counter);
  json_counter = 0;
  WriteKeyValue<FieldNumber>(stream, serialization_size, key, value, &json,
                             json_counter);
}
}  // namespace lightstep
