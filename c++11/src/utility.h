#ifndef LIGHTSTEP_UTILITY_H
#define LIGHTSTEP_UTILITY_H

#include <collector.pb.h>
#include <opentracing/stringref.h>
#include <opentracing/value.h>
#include <string>

namespace lightstep {
uint64_t generate_id();
std::string get_program_name();
collector::KeyValue to_key_value(opentracing::string_view key,
                                 const opentracing::Value& value);
}  // namespace lightstep

#endif  // LIGHTSTEP_UTILITY_H
