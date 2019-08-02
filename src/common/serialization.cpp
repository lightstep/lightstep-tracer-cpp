#include "common/serialization.h"

const size_t KeyValueKeyField = 1;
const size_t KeyValueStringValueField = 2;
const size_t KeyValueIntValueField = 3;
const size_t KeyValueDoubleValueField = 4;
const size_t KeyValueBoolValueField = 5;
const size_t KeyValueJsonValueField = 6;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// SerializationSizeValueVisitor
//--------------------------------------------------------------------------------------------------
namespace {
struct SerializationSizeValueVisitor {
  const opentracing::Value& original_value;
  std::string& json;
  int& json_counter;
  size_t& result;

  void operator()(bool value) const noexcept {
    result += ComputeVarintSerializationSize<KeyValueBoolValueField>(
        static_cast<uint32_t>(value));
  }

  void operator()(double /*value*/) const noexcept {
    result += StaticKeySerializationSize<KeyValueDoubleValueField,
                                         WireType::Fixed64>::value +
              sizeof(int64_t);
  }

  void operator()(int64_t value) const noexcept {
    result += ComputeVarintSerializationSize<KeyValueIntValueField>(
        static_cast<uint64_t>(value));
  }

  void operator()(uint64_t value) const noexcept {
    // There's no uint64_t value type so cast to an int64_t.
    this->operator()(static_cast<int64_t>(value));
  }

  void operator()(opentracing::string_view s) const noexcept {
    result += ComputeLengthDelimitedSerializationSize<KeyValueStringValueField>(
        s.size());
  }

  void operator()(const std::string& s) const noexcept {
    this->operator()(opentracing::string_view{s});
  }

  void operator()(std::nullptr_t) const noexcept { this->operator()(false); }

  void operator()(const char* s) const noexcept {
    this->operator()(opentracing::string_view{s});
  }

  void operator()(const opentracing::Values& /*unused*/) const { do_json(); }

  void operator()(const opentracing::Dictionary& /*unused*/) const {
    do_json();
  }

  void do_json() const {
    json = ToJson(original_value);
    ++json_counter;
    result += ComputeLengthDelimitedSerializationSize<KeyValueJsonValueField>(
        json.size());
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// SerializationValueVisitor
//--------------------------------------------------------------------------------------------------
namespace {
struct SerializationValueVisitor {
  google::protobuf::io::CodedOutputStream& stream;
  const std::string* json_values;
  int& json_counter;

  void operator()(bool value) const {
    WriteVarint<KeyValueBoolValueField>(stream, static_cast<uint32_t>(value));
  }

  void operator()(double value) const {
    WriteFixed64<KeyValueDoubleValueField>(stream, static_cast<void*>(&value));
  }

  void operator()(int64_t value) const {
    WriteVarint<KeyValueIntValueField>(stream, static_cast<uint64_t>(value));
  }

  void operator()(uint64_t value) const {
    // There's no uint64_t value type so cast to an int64_t.
    this->operator()(static_cast<int64_t>(value));
  }

  void operator()(opentracing::string_view s) const {
    WriteString<KeyValueStringValueField>(stream, s);
  }

  void operator()(const std::string& s) const {
    this->operator()(opentracing::string_view{s});
  }

  void operator()(std::nullptr_t) const { this->operator()(false); }

  void operator()(const char* s) const {
    this->operator()(opentracing::string_view{s});
  }

  void operator()(const opentracing::Values& /*unused*/) const { do_json(); }

  void operator()(const opentracing::Dictionary& /*unused*/) const {
    do_json();
  }

  void do_json() const {
    WriteString<KeyValueJsonValueField>(stream, json_values[json_counter]);
    ++json_counter;
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// ComputeKeyValueSerializationSize
//--------------------------------------------------------------------------------------------------
size_t ComputeKeyValueSerializationSize(opentracing::string_view key,
                                        const opentracing::Value& value,
                                        std::string& json, int& json_counter) {
  size_t result =
      ComputeLengthDelimitedSerializationSize<KeyValueKeyField>(key.size());
  SerializationSizeValueVisitor value_visitor{value, json, json_counter,
                                              result};
  apply_visitor(value_visitor, value);
  return result;
}

//--------------------------------------------------------------------------------------------------
// WriteKeyValueImpl
//--------------------------------------------------------------------------------------------------
void WriteKeyValueImpl(google::protobuf::io::CodedOutputStream& stream,
                       opentracing::string_view key,
                       const opentracing::Value& value,
                       const std::string* json_values, int& json_counter) {
  WriteString<KeyValueKeyField>(stream, key);
  SerializationValueVisitor value_visitor{stream, json_values, json_counter};
  apply_visitor(value_visitor, value);
}
}  // namespace lightstep
