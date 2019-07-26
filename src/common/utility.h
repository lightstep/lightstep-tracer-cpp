#pragma once

#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <chrono>
#include <cstdint>
#include <limits>
#include <string>
#include "common/logger.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <sys/time.h>

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
const size_t Num64BitHexDigits = std::numeric_limits<uint64_t>::digits / 4;
const size_t Num32BitHexDigits = std::numeric_limits<uint32_t>::digits / 4;

/**
 * Breaks the timestamp down into seconds past epoch and nanosecond fraction
 * to match that used by google in protobuf.
 * See
 * https://github.com/protocolbuffers/protobuf/blob/8489612dadd3775ffbba029a583b6f00e91d0547/src/google/protobuf/timestamp.proto
 * @param t the time point to format
 */
std::tuple<uint64_t, uint32_t> ProtobufFormatTimestamp(
    const std::chrono::system_clock::time_point& t);

// Convert a std::chrono::system_clock::time_point to the time value used
// by protobuf.
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t);

timeval ToTimeval(std::chrono::microseconds microseconds);

template <class Rep, class Period>
inline timeval toTimeval(std::chrono::duration<Rep, Period> duration) {
  return toTimeval(
      std::chrono::duration_cast<std::chrono::microseconds>(duration));
}

// Attempts to determine the name of the executable invoked.  Returns
// "c++-program" if unsuccessful.
std::string GetProgramName();

/**
 * Converts an opentracing value to json.
 * @param value the value to convert
 * @return a json representation of value
 */
std::string ToJson(const opentracing::Value& value);

// Converts an OpenTracing key-value pair to the key-value pair used in the
// protobuf data structures.
collector::KeyValue ToKeyValue(opentracing::string_view key,
                               const opentracing::Value& value);

/**
 * Creates a protobuf log record.
 * @param timestamp the timestamp for the log record
 * @param field_first the first iterator for the log fields
 * @param field_last the last iterator for the log fields
 * @return the protobuf log record
 */
template <class Iterator>
collector::Log ToLog(std::chrono::system_clock::time_point timestamp,
                     Iterator field_first, Iterator field_last) {
  collector::Log result;
  *result.mutable_timestamp() = ToTimestamp(timestamp);
  auto& key_values = *result.mutable_fields();
  key_values.Reserve(static_cast<int>(std::distance(field_first, field_last)));
  for (Iterator field_iter = field_first; field_iter != field_last;
       ++field_iter) {
    *key_values.Add() = ToKeyValue(field_iter->first, field_iter->second);
  }
  return result;
}

// Logs any information returned by the collector.
void LogReportResponse(Logger& logger, bool verbose,
                       const collector::ReportResponse& response);

/**
 * Writes a 64-bit number in hex.
 * @param x the number to write
 * @param output where to output the number
 * @return x as a hex string
 */
opentracing::string_view Uint64ToHex(uint64_t x, char* output);

/**
 * Writes a 32-bit number in hex.
 * @param x the number to write
 * @param output where to output the number
 * @return x as a hex string
 */
opentracing::string_view Uint32ToHex(uint32_t x, char* output);

// Converts a hexidecimal number to a 64-bit integer. Either returns the number
// or an error code.
opentracing::expected<uint64_t> HexToUint64(opentracing::string_view s);

/**
 * Reads the header for an http/1.1 streaming chunk.
 * @param stream supplies the ZeroCopyInputStream to read from.
 * @param chunk_size is where the chunk's size is written.
 * @return true if the header was successfully read.
 */
bool ReadChunkHeader(google::protobuf::io::ZeroCopyInputStream& stream,
                     size_t& chunk_size);
}  // namespace lightstep
