#include "tracer/serialization.h"

#include "common/serialization.h"
#include "common/utility.h"

const size_t SpanContextField = 1;
const size_t OperationNameField = 2;
const size_t SpanReferenceField = 3;
const size_t StartTimestampField = 4;
const size_t DurationField = 5;
const size_t TagsField = 6;

const size_t SpanContextTraceIdField = 1;
const size_t SpanContextSpanIdField = 2;
const size_t SpanContextBaggageField = 3;

const size_t SpanReferenceRelationshipField = 1;
const size_t SpanReferenceSpanContextField = 2;

const size_t MapEntryKeyField = 1;
const size_t MapEntryValueField = 2;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ComputeSpanContextSerializationSize
//--------------------------------------------------------------------------------------------------
static size_t ComputeSpanContextSerializationSize(
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
// WriteSpanContext
//--------------------------------------------------------------------------------------------------
template <size_t FieldNumber>
static void WriteSpanContext(
    google::protobuf::io::CodedOutputStream& stream, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage = {}) {
  auto serialization_size =
      ComputeSpanContextSerializationSize(trace_id, span_id, baggage);
  SerializeKeyLength<FieldNumber>(stream, serialization_size);
  SerializeVarint<SpanContextTraceIdField>(stream, trace_id);
  SerializeVarint<SpanContextSpanIdField>(stream, span_id);
  for (auto& baggage_item : baggage) {
    auto baggage_serialization_size =
        ComputeLengthDelimitedSerializationSize<MapEntryKeyField>(
            baggage_item.first.size()) +
        ComputeLengthDelimitedSerializationSize<MapEntryValueField>(
            baggage_item.second.size());
    SerializeKeyLength<SpanContextBaggageField>(stream,
                                                baggage_serialization_size);
    SerializeString<MapEntryKeyField>(stream, baggage_item.first);
    SerializeString<MapEntryValueField>(stream, baggage_item.second);
  }
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
  SerializeKeyValue<TagsField>(stream, key, value);
}

//--------------------------------------------------------------------------------------------------
// WriteStartTimestamp
//--------------------------------------------------------------------------------------------------
void WriteStartTimestamp(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp) {
  SerializeTimestamp<StartTimestampField>(stream, timestamp);
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
  auto serialization_size =
      ComputeVarintSerializationSize<SpanReferenceRelationshipField>(
          relationship) +
      ComputeLengthDelimitedSerializationSize<SpanReferenceSpanContextField>(
          ComputeSpanContextSerializationSize(trace_id, span_id));
  SerializeKeyLength<SpanReferenceField>(stream, serialization_size);
  SerializeVarint<SpanReferenceRelationshipField>(stream, relationship);
  WriteSpanContext<SpanReferenceSpanContextField>(stream, trace_id, span_id);
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
void WriteSpanContext(
    google::protobuf::io::CodedOutputStream& stream, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage) {
  WriteSpanContext<SpanContextField>(stream, trace_id, span_id, baggage);
}
}  // namespace lightstep
