#include "tracer/serialization.h"

#include "common/serialization.h"
#include "common/utility.h"

const size_t OperationNameField = 2;
const size_t StartTimestampField = 4;
const size_t DurationField = 5;

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

  void operator()(bool value) const {
    (void)value;
    result = 0;
  }

  void operator()(double value) const {
    (void)value;
    result = 0;
  }

  void operator()(int64_t value) const {
    (void)value;
    result = 0;
  }

  void operator()(uint64_t value) const {
    // There's no uint64_t value type so cast to an int64_t.
    this->operator()(static_cast<int64_t>(value));
  }

  void operator()(opentracing::string_view s) const {
    (void)s;
    result = 0;
  }

  void operator()(const std::string& s) const {
    this->operator()(opentracing::string_view{s});
    result = 0;
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
    json = ToJson(original_value);
    ++json_counter;
    result = 0;
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// SerializationValueVisitor
//--------------------------------------------------------------------------------------------------
namespace {
struct SerializationValueVisitor {
  google::protobuf::io::CodedOutputStream& stream;
  const std::string& json;
  int& json_counter;

  void operator()(bool value) const { (void)value; }

  void operator()(double value) const { (void)value; }

  void operator()(int64_t value) const { (void)value; }

  void operator()(uint64_t value) const {
    // There's no uint64_t value type so cast to an int64_t.
    this->operator()(static_cast<int64_t>(value));
  }

  void operator()(opentracing::string_view s) const { (void)s; }

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

  void do_json() const { ++json_counter; }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// ComputeValueSerializationSize
//--------------------------------------------------------------------------------------------------
static size_t ComputeValueSerializationSize(const opentracing::Value& value,
                                     std::string& json, int& json_counter) {
  size_t result;
  SerializationSizeValueVisitor value_visitor{value, json, json_counter,
                                              result};
  apply_visitor(value_visitor, value);
  return result;
}

//--------------------------------------------------------------------------------------------------
// SerializeValue
//--------------------------------------------------------------------------------------------------
static void SerializeValue(google::protobuf::io::CodedOutputStream& stream,
                    const opentracing::Value& value, const std::string& json,
                    int& json_counter) {
  SerializationValueVisitor value_visitor{stream, json, json_counter};
  apply_visitor(value_visitor, value);
}

//--------------------------------------------------------------------------------------------------
// WriteOperationName
//--------------------------------------------------------------------------------------------------
void WriteOperationName(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::string_view operation_name) {
  SerializeString<OperationNameField>(stream, operation_name);
}

//--------------------------------------------------------------------------------------------------
// WriteTag
//--------------------------------------------------------------------------------------------------
void WriteTag(google::protobuf::io::CodedOutputStream& stream,
              opentracing::string_view key, const opentracing::Value& value) {
  (void)ComputeValueSerializationSize;
  (void)SerializeValue;
  (void)stream;
  (void)key;
  (void)value;
}

//--------------------------------------------------------------------------------------------------
// WriteStartTimestamp
//--------------------------------------------------------------------------------------------------
void WriteStartTimestamp(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp) {
  SerializeTimestamp<StartTimestampField>(stream, timestamp);
}

//--------------------------------------------------------------------------------------------------
// WriteDuration
//--------------------------------------------------------------------------------------------------
void WriteDuration(google::protobuf::io::CodedOutputStream& stream,
                   std::chrono::steady_clock::duration duration) {
  SerializeVarint<DurationField>(
      stream,
      static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::microseconds>(duration)
              .count()));
}

//--------------------------------------------------------------------------------------------------
// WriteLog
//--------------------------------------------------------------------------------------------------
template <class Iterator>
void WriteLog(google::protobuf::io::CodedOutputStream& stream,
              opentracing::SystemTime timestamp, Iterator first,
              Iterator last) {
  (void)stream;
  (void)timestamp;
  (void)first;
  (void)last;
}

//--------------------------------------------------------------------------------------------------
// WriteSpanContext
//--------------------------------------------------------------------------------------------------
void WriteSpanContext(google::protobuf::io::CodedOutputStream& stream,
                      uint64_t trace_id, uint64_t span_id,
                      const std::vector<std::string, std::string>& baggage) {
  (void)stream;
  (void)trace_id;
  (void)span_id;
  (void)baggage;
}
}  // namespace lightstep
