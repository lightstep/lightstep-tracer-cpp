#include "tracer/serialization.h"

#include "common/serialization.h"

const size_t OperationNameKey = 2; 
const size_t DurationKey = 5;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// WriteOperationName
//--------------------------------------------------------------------------------------------------
void WriteOperationName(google::protobuf::io::CodedOutputStream& stream,
                        opentracing::string_view operation_name) {
  SerializeString<OperationNameKey>(stream, operation_name);
}

//--------------------------------------------------------------------------------------------------
// WriteTag
//--------------------------------------------------------------------------------------------------
void WriteTag(google::protobuf::io::CodedOutputStream& stream, opentracing::string_view key,
    const opentracing::Value& value) {
  (void)stream;
  (void)key;
  (void)value;
}

//--------------------------------------------------------------------------------------------------
// WriteStartTimestamp
//--------------------------------------------------------------------------------------------------
void WriteStartTimestamp(google::protobuf::io::CodedOutputStream& stream,
                         opentracing::SystemTime timestamp) {
  (void)stream;
  (void)timestamp;
}

//--------------------------------------------------------------------------------------------------
// WriteDuration
//--------------------------------------------------------------------------------------------------
void WriteDuration(google::protobuf::io::CodedOutputStream& stream,
                   std::chrono::steady_clock::duration duration) {
  SerializeVarint<DurationKey>(
      stream,
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

//--------------------------------------------------------------------------------------------------
// WriteLog
//--------------------------------------------------------------------------------------------------
template <class Iterator>
void WriteLog(google::protobuf::io::CodedOutputStream& stream,
              opentracing::SystemTime timestamp, Iterator first, Iterator last) {
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
} // namespace lightstep
