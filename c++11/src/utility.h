#pragma once

#include <collector.pb.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <chrono>
#include <string>

namespace lightstep {
/**
 * Convert a std::chrono::system_clock::time_point to the time value used
 * by protobuf.
 */
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t);

/**
 * Generates a random uint64_t.
 */
uint64_t GenerateId();

/**
 * Attempts to determine the name of the executable invoked.  Returns
 * "c++-program" if unsuccessful.
 */
std::string GetProgramName();

/**
 * Converts an OpenTracing key-value pair to the key-value pair used in the
 * protobuf data structures.
 */
collector::KeyValue to_key_value(opentracing::string_view key,
                                 const opentracing::Value& value);
}  // namespace lightstep
