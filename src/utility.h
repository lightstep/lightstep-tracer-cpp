#pragma once

#include <netdb.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <algorithm>
#include <chrono>
#include <string>
#include <type_traits>

#include "lightstep-tracer-common/collector.pb.h"
#include "logger.h"

namespace lightstep {
// Converts a network address to a readable string
std::string AddressToString(const sockaddr& address);

// Convert a std::chrono::system_clock::time_point to the time value used
// by protobuf.
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t);

// Generates a random uint64_t.
uint64_t GenerateId();

// Attempts to determine the name of the executable invoked.  Returns
// "c++-program" if unsuccessful.
std::string GetProgramName();

// Converts an OpenTracing key-value pair to the key-value pair used in the
// protobuf data structures.
collector::KeyValue ToKeyValue(opentracing::string_view key,
                               const opentracing::Value& value);

// Logs any information returned by the collector.
void LogReportResponse(Logger& logger, bool verbose,
                       const collector::ReportResponse& response);

// Reverses the endianness of an integral value.
template <class T>
inline void ReverseEndianness(T& t) {
  static_assert(std::is_integral<T>::value, "Must be integral type");
  char* data = reinterpret_cast<char*>(&t);
  std::reverse(data, data + sizeof(T));
}

// Converts `x` to a hexidecimal, writes the results into `output` and returns
// a string_view of the number.
opentracing::string_view Uint64ToHex(uint64_t x, char* output);

// Converts a hexidecimal number to a 64-bit integer. Either returns the number
// or an error code.
opentracing::expected<uint64_t> HexToUint64(opentracing::string_view s);
}  // namespace lightstep
