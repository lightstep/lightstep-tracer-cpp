#pragma once

#include <string>
#include <utility>
#include <vector>

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/propagation.h>
#include <opentracing/span.h>
#include <opentracing/string_view.h>

namespace lightstep {
/**
 * Serializes the operation name of a span.
 * @param stream the stream to serialize into
 * @param operation_name the operation name to serialize
 */
void WriteOperationName(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::string_view operation_name);

/**
 * Serializes a tag for a span.
 * @param stream the stream to serialize into
 * @param key the key for the tag
 * @param value the value of the tag
 */
void WriteTag(google::protobuf::io::CodedOutputStream& stream,
              opentracing::string_view key, const opentracing::Value& value);

/**
 * Serializes the start timestamp of a span.
 * @param stream the stream to serialize into
 * @param timestamp the start timestamp to serialize
 */
void WriteStartTimestamp(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp);

/**
 * Serialize a span reference.
 * @param stream the stream to serialize into
 * @param reference_type the type of reference
 * @param trace_id the trace id of the reference
 * @param span_id the span id of the reference
 */
void WriteSpanReference(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::SpanReferenceType reference_type,
                        uint64_t trace_id, uint64_t span_id);

/**
 * Serialize the duration of a span.
 * @param stream the stream to serialize into
 * @param duration the duration to serialize
 */
void WriteDuration(google::protobuf::io::CodedOutputStream& stream,
                   std::chrono::steady_clock::duration duration);

/**
 * Serialize a log record into a span.
 * @param stream the stream to serialize into
 * @param timestamp the timestamp of the log
 * @param first the start of the log record's fields
 * @param last the end of the log record's fields
 */
void WriteLog(
    google::protobuf::io::CodedOutputStream& stream,
    opentracing::SystemTime timestamp,
    const std::pair<opentracing::string_view, opentracing::Value>* first,
    const std::pair<opentracing::string_view, opentracing::Value>* last);

/**
 * Serialize a log record into a span.
 * @param stream the stream to serialize into
 * @param timestamp the timestamp of the log
 * @param first the start of the log record's fields
 * @param last the end of the log record's fields
 */
void WriteLog(google::protobuf::io::CodedOutputStream& stream,
              opentracing::SystemTime timestamp,
              const std::pair<std::string, opentracing::Value>* first,
              const std::pair<std::string, opentracing::Value>* last);

/**
 * Serialize a span context into a span.
 * @param stream the stream to serialize into
 * @param trace_id the trace id of the span's context
 * @param span_id the span id of the span's context
 * @param baggage the baggage attached to the span context
 */
void WriteSpanContext(
    google::protobuf::io::CodedOutputStream& stream, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage);
}  // namespace lightstep
