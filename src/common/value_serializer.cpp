#include "common/value_serializer.h"

#include "common/serialization.h"

const size_t KeyValueStringValueField = 2;
const size_t KeyValueIntValueField = 3;
const size_t KeyValueDoubleValueField = 4;
const size_t KeyValueBoolValueField = 5;
const size_t KeyValueJsonValueField = 6;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// SerializeStringValue
//--------------------------------------------------------------------------------------------------
template <class Stream>
static void SerializeStringValue(Stream& stream,
                                 const ValueSerializerData& data) {
  WriteString<KeyValueStringValueField>(stream, data.string_view_value);
}

//--------------------------------------------------------------------------------------------------
// SerializeBoolValue
//--------------------------------------------------------------------------------------------------
template <class Stream>
static void SerializeBoolValue(Stream& stream,
                               const ValueSerializerData& data) {
  WriteVarint<KeyValueBoolValueField>(stream,
                                      static_cast<uint32_t>(data.bool_value));
}

//--------------------------------------------------------------------------------------------------
// SerializeIntValue
//--------------------------------------------------------------------------------------------------
template <class Stream>
static void SerializeIntValue(Stream& stream, const ValueSerializerData& data) {
  WriteVarint<KeyValueIntValueField>(stream,
                                     static_cast<uint64_t>(data.int64_value));
}

//--------------------------------------------------------------------------------------------------
// SerializeDoubleValue
//--------------------------------------------------------------------------------------------------
template <class Stream>
static void SerializeDoubleValue(Stream& stream,
                                 const ValueSerializerData& data) {
  WriteFixed64<KeyValueDoubleValueField>(
      stream, static_cast<const void*>(&data.double_value));
}

//--------------------------------------------------------------------------------------------------
// SerializeJsonValue
//--------------------------------------------------------------------------------------------------
template <class Stream>
static void SerializeJsonValue(Stream& stream,
                               const ValueSerializerData& data) {
  WriteString<KeyValueJsonValueField>(stream, data.string_view_value);
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ValueSerializer::ValueSerializer(opentracing::string_view value) noexcept {
  data_.string_view_value = value;
  serializer_ = SerializeStringValue<google::protobuf::io::CodedOutputStream>;
  direct_serializer_ = SerializeStringValue<DirectCodedOutputStream>;
}

ValueSerializer::ValueSerializer(bool value) noexcept {
  data_.bool_value = value;
  serializer_ = SerializeBoolValue<google::protobuf::io::CodedOutputStream>;
  direct_serializer_ = SerializeBoolValue<DirectCodedOutputStream>;
}

ValueSerializer::ValueSerializer(int64_t value) noexcept {
  data_.int64_value = value;
  serializer_ = SerializeIntValue<google::protobuf::io::CodedOutputStream>;
  direct_serializer_ = SerializeIntValue<DirectCodedOutputStream>;
}

ValueSerializer::ValueSerializer(double value) noexcept {
  data_.double_value = value;
  serializer_ = SerializeDoubleValue<google::protobuf::io::CodedOutputStream>;
  direct_serializer_ = SerializeDoubleValue<DirectCodedOutputStream>;
}

ValueSerializer::ValueSerializer(json_tag,
                                 opentracing::string_view value) noexcept {
  data_.string_view_value = value;
  serializer_ = SerializeJsonValue<google::protobuf::io::CodedOutputStream>;
  direct_serializer_ = SerializeJsonValue<DirectCodedOutputStream>;
}
}  // namespace lightstep
