#pragma once

#include <string>
#include <utility>
#include <vector>

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/propagation.h>
#include <opentracing/span.h>
#include <opentracing/string_view.h>

namespace lightstep {
void WriteOperationName(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::string_view operation_name);

void WriteTag(google::protobuf::io::CodedOutputStream& stream,
              opentracing::string_view key, const opentracing::Value& value);

void WriteStartTimestamp(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp);

void WriteSpanReference(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::SpanReferenceType reference_type,
                        uint64_t trace_id, uint64_t span_id);

void WriteDuration(google::protobuf::io::CodedOutputStream& stream,
                   std::chrono::steady_clock::duration duration);

void WriteLog(
    google::protobuf::io::CodedOutputStream& stream,
    opentracing::SystemTime timestamp,
    const std::pair<opentracing::string_view, opentracing::Value>* first,
    const std::pair<opentracing::string_view, opentracing::Value>* last);

void WriteLog(google::protobuf::io::CodedOutputStream& stream,
              opentracing::SystemTime timestamp,
              const std::pair<std::string, opentracing::Value>* first,
              const std::pair<std::string, opentracing::Value>* last);

void WriteSpanContext(
    google::protobuf::io::CodedOutputStream& stream, uint64_t trace_id,
    uint64_t span_id,
    const std::vector<std::pair<std::string, std::string>>& baggage);
}  // namespace lightstep
