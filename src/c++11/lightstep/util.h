// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_UTIL_H__
#define __LIGHTSTEP_UTIL_H__

#include <chrono>
#include <string>
#include "collector.pb.h"

namespace lightstep {

typedef std::chrono::system_clock Clock;
typedef Clock::time_point TimeStamp;
typedef Clock::duration Duration;

namespace util {

std::string program_name();

google::protobuf::Timestamp to_timestamp(TimeStamp t);

template <typename T>
collector::KeyValue make_kv(const std::string& key, const T& value);

template <>
collector::KeyValue make_kv(const std::string& key, const uint64_t& value);

uint64_t hexToUint64(const std::string& s);

}  // namespace util 
}  // namespace lightstep

#endif // __LIGHTSTEP_UTIL_H__
