#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>

#include "common/direct_coded_output_stream.h"
#include "common/utility.h"

namespace lightstep {
// Wire type values that matches those used by protobuf.
// See https://developers.google.com/protocol-buffers/docs/encoding#structure
// Only lists wire types we use
enum class WireType { Varint = 0, Fixed64 = 1, LengthDelimited = 2 };

/**
 * Compute the encoding of a field and its type at compile-time
 */
template <size_t FieldNumber, WireType WireTypeValue>
struct StaticSerializationKey {
  // See https://developers.google.com/protocol-buffers/docs/encoding#structure
  // for documentation on encoding.
  static constexpr uint32_t value = static_cast<uint32_t>(
      (FieldNumber << 3) | static_cast<size_t>(WireTypeValue));
};

/**
 * Compute the size of an encoded field and its type at compile-time.
 */
template <size_t FieldNumber, WireType WireTypeValue>
struct StaticKeySerializationSize {
  static constexpr size_t value =
      google::protobuf::io::CodedOutputStream::StaticVarintSize32<
          StaticSerializationKey<FieldNumber, WireTypeValue>::value>::value;
};

/**
 * Compute the serialization size of a varint with its key field.
 * @param x the varint
 * @return the serialization size
 */
template <size_t FieldNumber>
inline size_t ComputeVarintSerializationSize(uint64_t x) noexcept {
  return StaticKeySerializationSize<FieldNumber, WireType::Varint>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(x);
}

/**
 * Compute the serialization size of a varint with its key field.
 * @param x the varint
 * @return the serialization size
 */
template <size_t FieldNumber>
inline size_t ComputeVarintSerializationSize(uint32_t x) noexcept {
  return StaticKeySerializationSize<FieldNumber, WireType::Varint>::value +
         google::protobuf::io::CodedOutputStream::VarintSize32(x);
}

/**
 * Compute the serialization size of a length-delimited protobuf message header.
 * @param length the length of the protobuf message
 * @return the serialization size
 */
template <size_t FieldNumber>
inline size_t ComputeLengthDelimitedHeaderSerializationSize(
    size_t length) noexcept {
  return StaticKeySerializationSize<FieldNumber,
                                    WireType::LengthDelimited>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(length);
}

/**
 * Compute the serialization size of a length-delimited protobuf message with
 * its key field.
 * @param length the length of the protobuf message
 * @return the serialization size
 */
template <size_t FieldNumber>
inline size_t ComputeLengthDelimitedSerializationSize(size_t length) noexcept {
  return StaticKeySerializationSize<FieldNumber,
                                    WireType::LengthDelimited>::value +
         google::protobuf::io::CodedOutputStream::VarintSize64(length) + length;
}

/**
 * Wrapper function that allows us to optimize the case of serializing a big
 * varint for DirectCodedOutputStream.
 * @param stream the stream to serialize into
 * @param x the integer to serialize
 */
inline void WriteBigVarint64(google::protobuf::io::CodedOutputStream& stream,
                             uint64_t x) {
  stream.WriteVarint64(x);
}

/**
 * Serialize a length-delimited key field with its length.
 * @param stream the stream to serialize into
 * @param the length for the field
 */
template <size_t FieldNumber, class Stream>
inline void WriteKeyLength(Stream& stream, size_t length) {
  stream.WriteTag(
      StaticSerializationKey<FieldNumber, WireType::LengthDelimited>::value);
  stream.WriteVarint32(static_cast<uint32_t>(length));
}

/**
 * Serialize a 64-bit fixed size field.
 * @param stream the stream to serialize into
 * @param data the data to serialize
 */
template <size_t FieldNumber, class Stream>
inline void WriteFixed64(Stream& stream, const void* data) {
  stream.WriteTag(
      StaticSerializationKey<FieldNumber, WireType::Fixed64>::value);
  stream.WriteRaw(data, sizeof(int64_t));
}

/**
 * Serialize a varint.
 * @param stream the stream to serialize into
 * @param x the value to serialize
 */
template <size_t FieldNumber, class Stream>
inline void WriteVarint(Stream& stream, uint64_t x) noexcept {
  stream.WriteTag(StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  stream.WriteVarint64(x);
}

/**
 * Serialize a varint.
 * @param stream the stream to serialize into
 * @param x the value to serialize
 */
template <size_t FieldNumber, class Stream>
inline void WriteVarint(Stream& stream, uint32_t x) {
  stream.WriteTag(StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  stream.WriteVarint32(x);
}

/**
 * Serialize a varint.
 * @param stream the stream to serialize into
 * @param x the value to serialize
 */
template <size_t FieldNumber, class Stream>
void WriteBigVarint(Stream& stream, uint64_t x) {
  stream.WriteTag(StaticSerializationKey<FieldNumber, WireType::Varint>::value);
  WriteBigVarint64(stream, x);
}

/**
 * Compute the serialization size of a key-value not including its key field.
 * @param key the key of the serialization
 * @param value the value of the serialization
 * @param json the string to write json into if the value requires conversion
 * @param json_counter an integer to increment if json is set
 * @return the serialization size
 */
size_t ComputeKeyValueSerializationSize(opentracing::string_view key,
                                        const opentracing::Value& value,
                                        std::string& json, int& json_counter);

/**
 * Serialize a key-value not including its key field.
 * @param stream the stream to serialize into
 * @param key the key of the serialization
 * @param value the value of the serialization
 * @param json_values an array of json values where json_values[json_counter]
 * references the json for this field if it's used.
 * @param json_counter an integer to increment if json is referenced
 */
void WriteKeyValueImpl(google::protobuf::io::CodedOutputStream& stream,
                       opentracing::string_view key,
                       const opentracing::Value& value,
                       const std::string* json_values, int& json_counter);

/**
 * Serialize a key-value with its key field.
 * @param stream the stream to serialize into
 * @param key the key of the serialization
 * @param value the value of the serialization
 * @param json_values an array of json values where json_values[json_counter]
 * references the json for this field if it's used.
 * @param json_counter an integer to increment if json is referenced
 */
template <size_t FieldNumber>
inline void WriteKeyValue(google::protobuf::io::CodedOutputStream& stream,
                          size_t serialization_size,
                          opentracing::string_view key,
                          const opentracing::Value& value,
                          const std::string* json_values, int& json_counter) {
  WriteKeyLength<FieldNumber>(stream, serialization_size);
  WriteKeyValueImpl(stream, key, value, json_values, json_counter);
}

/**
 * Serialize a key-value with its key field.
 * @param stream the stream to serialize into
 * @param key the key of the serialization
 * @param value the value of the serialization
 */
template <size_t FieldNumber>
inline void WriteKeyValue(google::protobuf::io::CodedOutputStream& stream,
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

/**
 * Serialize a length-delimited field and value into a stream.
 * @param stream the stream to serialize into
 * @param serialization_size the size of the serialization excluding the field
 * header
 * @param serializer a functor that performs the serialization
 * @param ...args arguments to forward to the serializer
 */
template <size_t FieldNumber, class Serializer, class... Args>
inline void WriteLengthDelimitedField(
    google::protobuf::io::CodedOutputStream& stream, size_t serialization_size,
    Serializer serializer, const Args&... args) {
  auto total_size =
      ComputeLengthDelimitedSerializationSize<FieldNumber>(serialization_size);
  auto buffer = stream.GetDirectBufferForNBytesAndAdvance(total_size);
  if (buffer != nullptr) {
    DirectCodedOutputStream direct_stream{buffer};
    WriteKeyLength<FieldNumber>(direct_stream, serialization_size);
    serializer(direct_stream, args...);
  } else {
    WriteKeyLength<FieldNumber>(stream, serialization_size);
    serializer(stream, args...);
  }
}

/**
 * Serialize a length-delimited field and value into a stream.
 * @param stream the stream to serialize into
 * @param serialization_size the size of the serialization excluding the field
 * header
 * @param serializer a functor that performs the serialization
 * @param ...args arguments to forward to the serializer
 */
template <size_t FieldNumber, class Serializer, class... Args>
inline void WriteLengthDelimitedField(DirectCodedOutputStream& stream,
                                      size_t serialization_size,
                                      Serializer serializer,
                                      const Args&... args) noexcept {
  WriteKeyLength<FieldNumber>(stream, serialization_size);
  serializer(stream, args...);
}

/**
 * Serialize the contents of a string into an arbitrary Stream.
 */
struct StringSerializer {
  template <class Stream>
  inline void operator()(Stream& stream, opentracing::string_view s) const {
    stream.WriteRaw(static_cast<const void*>(s.data()), s.size());
  }
};

/**
 * Serialize a string.
 * @param stream the stream to serialize into
 * @param s the string to serialize
 */
template <size_t FieldNumber, class Stream>
void WriteString(Stream& stream, opentracing::string_view s) {
  WriteLengthDelimitedField<FieldNumber>(stream, s.size(), StringSerializer{},
                                         s);
}

/**
 * Compute the serialization of a timestamp not including its key field.
 * @param seconds_since_epoch the seconds since the epoch
 * @param nano_fraction the fraction of remaining nanoseconds
 */
inline size_t ComputeTimestampSerializationSize(
    uint64_t seconds_since_epoch, uint32_t nano_fraction) noexcept {
  static const size_t TimestampSecondsSinceEpochField = 1;
  static const size_t TimestampNanoFractionField = 2;
  return ComputeVarintSerializationSize<TimestampSecondsSinceEpochField>(
             seconds_since_epoch) +
         ComputeVarintSerializationSize<TimestampNanoFractionField>(
             nano_fraction);
}

/**
 * Serialize a timestamp not including its key field.
 * @param stream the stream to serialize into
 * @param seconds_since_epoch the seconds since the epoch
 * @param nano_fraction the fraction of remaining nanoseconds
 */
template <class Stream>
inline void WriteTimestampImpl(Stream& stream, uint64_t seconds_since_epoch,
                               uint32_t nano_fraction) {
  static const size_t TimestampSecondsSinceEpochField = 1;
  static const size_t TimestampNanoFractionField = 2;
  WriteVarint<TimestampSecondsSinceEpochField>(stream, seconds_since_epoch);
  WriteVarint<TimestampNanoFractionField>(stream, nano_fraction);
}

/**
 * Serialize a timestamp into an arbitrary stream.
 */
struct TimestampSerializer {
  template <class Stream>
  void operator()(Stream& stream, uint64_t seconds_since_epoch,
                  uint32_t nano_fraction) const {
    WriteTimestampImpl(stream, seconds_since_epoch, nano_fraction);
  }
};

/**
 * Serialize a timestamp including its key field.
 * @param stream the stream to serialize into
 * @param serialization_size the serialization size of the timestamp not
 * including its key field
 * @param seconds_since_epoch the seconds since the epoch
 * @param nano_fraction the fraction of remaining nanoseconds
 */
template <size_t FieldNumber, class Stream>
inline void WriteTimestamp(Stream& stream, size_t serialization_size,
                           uint64_t seconds_since_epoch,
                           uint32_t nano_fraction) noexcept {
  WriteLengthDelimitedField<FieldNumber>(stream, serialization_size,
                                         TimestampSerializer{},
                                         seconds_since_epoch, nano_fraction);
}

/**
 * Serialize a timestamp.
 * @param stream the stream to serialize into
 * @param timestamp the timestamp to serialize
 */
template <size_t FieldNumber>
inline void WriteTimestamp(google::protobuf::io::CodedOutputStream& stream,
                           std::chrono::system_clock::time_point timestamp) {
  uint64_t seconds_since_epoch;
  uint32_t nano_fraction;
  std::tie(seconds_since_epoch, nano_fraction) =
      ProtobufFormatTimestamp(timestamp);
  auto serialization_size =
      ComputeTimestampSerializationSize(seconds_since_epoch, nano_fraction);
  WriteTimestamp<FieldNumber>(stream, serialization_size, seconds_since_epoch,
                              nano_fraction);
}
}  // namespace lightstep
