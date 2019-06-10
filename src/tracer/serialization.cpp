#include "tracer/serialization.h"

#include "common/serialization.h"
#include "common/utility.h"

const size_t OperationNameField = 2;
const size_t StartTimestampField = 4;
const size_t DurationField = 5;
const size_t TagsField = 6;

namespace lightstep {
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
