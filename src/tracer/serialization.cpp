#include "tracer/serialization.h"

#include <iterator>

#include "common/platform/memory.h"
#include "common/serialization.h"
#include "common/utility.h"

const size_t SpanContextField = 1;
const size_t OperationNameField = 2;
const size_t SpanReferenceField = 3;
const size_t StartTimestampField = 4;
const size_t DurationField = 5;
const size_t TagsField = 6;
const size_t LogsField = 7;

const size_t SpanContextTraceIdField = 1;
const size_t SpanContextSpanIdField = 2;
const size_t SpanContextBaggageField = 3;

const size_t SpanReferenceRelationshipField = 1;
const size_t SpanReferenceSpanContextField = 2;

const size_t MapEntryKeyField = 1;
const size_t MapEntryValueField = 2;

const size_t LogTimestampField = 1;
const size_t LogFieldsField = 2;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ComputeSpanContextSerializationSize
//--------------------------------------------------------------------------------------------------
static inline size_t ComputeSpanContextSerializationSize(
    uint64_t trace_id, uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage =
        {}) noexcept {
  auto result =
      ComputeVarintSerializationSize<SpanContextTraceIdField>(trace_id) +
      ComputeVarintSerializationSize<SpanContextSpanIdField>(span_id);
  for (auto& baggage_item : baggage) {
    auto baggage_serialization_size =
        ComputeLengthDelimitedSerializationSize<MapEntryKeyField>(
            baggage_item.first.size()) +
        ComputeLengthDelimitedSerializationSize<MapEntryValueField>(
            baggage_item.second.size());
    result += ComputeLengthDelimitedSerializationSize<SpanContextBaggageField>(
        baggage_serialization_size);
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// WriteSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class Stream>
static inline void WriteSpanContextImpl(
    Stream& stream, uint64_t trace_id, uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage) {
  WriteBigVarint<SpanContextTraceIdField>(stream, trace_id);
  WriteBigVarint<SpanContextSpanIdField>(stream, span_id);
  for (auto& baggage_item : baggage) {
    auto baggage_serialization_size =
        ComputeLengthDelimitedSerializationSize<MapEntryKeyField>(
            baggage_item.first.size()) +
        ComputeLengthDelimitedSerializationSize<MapEntryValueField>(
            baggage_item.second.size());
    WriteKeyLength<SpanContextBaggageField>(stream, baggage_serialization_size);
    WriteString<MapEntryKeyField>(stream, baggage_item.first);
    WriteString<MapEntryValueField>(stream, baggage_item.second);
  }
}

//--------------------------------------------------------------------------------------------------
// SpanContextSerializer
//--------------------------------------------------------------------------------------------------
struct SpanContextSerializer {
  template <class Stream>
  inline void operator()(
      Stream& stream, uint64_t trace_id, uint64_t span_id,
      const std::vector<std::pair<std::string, std::string>>& baggage) const {
    WriteSpanContextImpl(stream, trace_id, span_id, baggage);
  }
};

//--------------------------------------------------------------------------------------------------
// WriteSpanContext
//--------------------------------------------------------------------------------------------------
template <size_t FieldNumber, class Stream>
static void WriteSpanContext(
    Stream& stream, size_t serialization_size, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage = {}) {
  WriteLengthDelimitedField<FieldNumber>(stream, serialization_size,
                                         SpanContextSerializer{}, trace_id,
                                         span_id, baggage);
}

template <size_t FieldNumber>
static void WriteSpanContext(
    google::protobuf::io::CodedOutputStream& stream, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage = {}) {
  auto serialization_size =
      ComputeSpanContextSerializationSize(trace_id, span_id, baggage);
  return WriteSpanContext<FieldNumber>(stream, serialization_size, trace_id,
                                       span_id, baggage);
}

//--------------------------------------------------------------------------------------------------
// WriteLogImpl
//--------------------------------------------------------------------------------------------------
template <class Iterator>
static void WriteLogImpl(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp, Iterator first,
                         Iterator last) {
  auto num_key_values = std::distance(first, last);

  auto field_serialization_sizes =
      static_cast<size_t*>(alloca(num_key_values * sizeof(size_t)));
  std::vector<std::string> json_values;
  std::string json;
  int json_counter = 0;

  uint64_t seconds_since_epoch;
  uint32_t nano_fraction;
  std::tie(seconds_since_epoch, nano_fraction) =
      ProtobufFormatTimestamp(timestamp);
  auto timestamp_serialization_size =
      ComputeTimestampSerializationSize(seconds_since_epoch, nano_fraction);

  size_t serialization_size =
      ComputeLengthDelimitedSerializationSize<LogTimestampField>(
          timestamp_serialization_size);

  int field_index = 0;
  for (auto iter = first; iter != last; ++iter) {
    auto field_serialization_size = ComputeKeyValueSerializationSize(
        iter->first, iter->second, json, json_counter);
    if (static_cast<size_t>(json_counter) > json_values.size()) {
      json_values.emplace_back(std::move(json));
    }
    field_serialization_sizes[field_index] = field_serialization_size;
    serialization_size +=
        ComputeLengthDelimitedSerializationSize<LogFieldsField>(
            field_serialization_size);
    ++field_index;
  }

  WriteKeyLength<LogsField>(stream, serialization_size);
  WriteTimestamp<LogTimestampField>(stream, timestamp_serialization_size,
                                    seconds_since_epoch, nano_fraction);
  field_index = 0;
  json_counter = 0;
  for (auto iter = first; iter != last; ++iter) {
    WriteKeyValue<LogFieldsField>(
        stream, field_serialization_sizes[field_index], iter->first,
        iter->second, json_values.data(), json_counter);
    ++field_index;
  }
}

//--------------------------------------------------------------------------------------------------
// WriteOperationName
//--------------------------------------------------------------------------------------------------
void WriteOperationName(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::string_view operation_name) {
  WriteString<OperationNameField>(stream, operation_name);
}

//--------------------------------------------------------------------------------------------------
// WriteTag
//--------------------------------------------------------------------------------------------------
void WriteTag(google::protobuf::io::CodedOutputStream& stream,
              opentracing::string_view key, const opentracing::Value& value) {
  WriteKeyValue<TagsField>(stream, key, value);
}

//--------------------------------------------------------------------------------------------------
// WriteStartTimestamp
//--------------------------------------------------------------------------------------------------
void WriteStartTimestamp(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp) {
  WriteTimestamp<StartTimestampField>(stream, timestamp);
}

//--------------------------------------------------------------------------------------------------
// WriteSpanReference
//--------------------------------------------------------------------------------------------------
void WriteSpanReference(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::SpanReferenceType reference_type,
                        uint64_t trace_id, uint64_t span_id) {
  static const uint32_t RelationshipChildOf = 0, RelationshipFollowsFrom = 1;
  auto relationship =
      reference_type == opentracing::SpanReferenceType::ChildOfRef
          ? RelationshipChildOf
          : RelationshipFollowsFrom;
  auto span_context_serialization_size =
      ComputeSpanContextSerializationSize(trace_id, span_id);
  auto serialization_size =
      ComputeVarintSerializationSize<SpanReferenceRelationshipField>(
          relationship) +
      ComputeLengthDelimitedSerializationSize<SpanReferenceSpanContextField>(
          span_context_serialization_size);
  WriteKeyLength<SpanReferenceField>(stream, serialization_size);
  WriteVarint<SpanReferenceRelationshipField>(stream, relationship);
  WriteSpanContext<SpanReferenceSpanContextField>(
      stream, span_context_serialization_size, trace_id, span_id);
}

//--------------------------------------------------------------------------------------------------
// WriteDuration
//--------------------------------------------------------------------------------------------------
void WriteDuration(google::protobuf::io::CodedOutputStream& stream,
                   std::chrono::steady_clock::duration duration) {
  auto elapse = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
  auto total_size = ComputeVarintSerializationSize<DurationField>(elapse);
  auto buffer = stream.GetDirectBufferForNBytesAndAdvance(total_size);
  if (buffer != nullptr) {
    DirectCodedOutputStream direct_stream{buffer};
    WriteVarint<DurationField>(direct_stream, elapse);
  } else {
    WriteVarint<DurationField>(stream, elapse);
  }
}

//--------------------------------------------------------------------------------------------------
// WriteLog
//--------------------------------------------------------------------------------------------------
void WriteLog(
    google::protobuf::io::CodedOutputStream& stream,
    opentracing::SystemTime timestamp,
    const std::pair<opentracing::string_view, opentracing::Value>* first,
    const std::pair<opentracing::string_view, opentracing::Value>* last) {
  WriteLogImpl(stream, timestamp, first, last);
}

void WriteLog(google::protobuf::io::CodedOutputStream& stream,
              opentracing::SystemTime timestamp,
              const std::pair<std::string, opentracing::Value>* first,
              const std::pair<std::string, opentracing::Value>* last) {
  WriteLogImpl(stream, timestamp, first, last);
}

//--------------------------------------------------------------------------------------------------
// WriteSpanContext
//--------------------------------------------------------------------------------------------------
void WriteSpanContext(
    google::protobuf::io::CodedOutputStream& stream, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage) {
  WriteSpanContext<SpanContextField>(stream, trace_id, span_id, baggage);
}
}  // namespace lightstep
