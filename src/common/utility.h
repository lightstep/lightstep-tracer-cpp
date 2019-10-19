#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <string>

#include "common/logger.h"
#include "common/platform/time.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>

namespace lightstep {
/**
 * Breaks the timestamp down into seconds past epoch and nanosecond fraction
 * to match that used by google in protobuf.
 * See
 * https://github.com/protocolbuffers/protobuf/blob/8489612dadd3775ffbba029a583b6f00e91d0547/src/google/protobuf/timestamp.proto
 * @param t the time point to format
 */
inline std::tuple<uint64_t, uint32_t> ProtobufFormatTimestamp(
    const std::chrono::system_clock::time_point& t) {
  auto nanos =
      std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch())
          .count();
  const uint64_t nanosPerSec = 1000000000;
  return {static_cast<uint64_t>(nanos / nanosPerSec),
          static_cast<uint32_t>(nanos % nanosPerSec)};
}

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
 * Reads the header for an http/1.1 streaming chunk.
 * @param stream supplies the ZeroCopyInputStream to read from.
 * @param chunk_size is where the chunk's size is written.
 * @return true if the header was successfully read.
 */
bool ReadChunkHeader(google::protobuf::io::ZeroCopyInputStream& stream,
                     size_t& chunk_size);

/**
 * std::error_code's have default comparison operators; however, they make use
 * of singleton addresses which can cause comparisons to fail when multiple
 * versions of the opentracing library are linked in. Since this is a common
 * deployment scenario when making OpenTracing plugins, we add this utility
 * function to make comparing std::error_code across libraries easier.
 *
 * Note: There's a proposed change to the C++ standard that addresses this
 * issue. See
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1196r0.html
 * @param lhs an std::error_code
 * @param rhs an std::error_code
 * @return true if the std::error_codes are equal
 */
inline bool AreErrorsEqual(std::error_code lhs, std::error_code rhs) noexcept {
  return opentracing::string_view{lhs.category().name()} ==
             opentracing::string_view{rhs.category().name()} &&
         lhs.value() == rhs.value();
}

/**
 * Converts a string to lower-case.
 * @param s a string
 * @return the lower-case version of s
 */
std::string ToLower(opentracing::string_view s);
}  // namespace lightstep
