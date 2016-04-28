// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_OPTIONS_H__
#define __LIGHTSTEP_OPTIONS_H__

#include "lightstep_thrift/lightstep_types.h"
#include <chrono>

#include "span.h"
#include "value.h"

namespace lightstep {

typedef std::chrono::system_clock Clock;
typedef Clock::time_point TimeStamp;

struct TracerOptions {
  TracerOptions()
    : should_sample([](uint64_t tid) { return true; }),
      binary_transport([](const uint8_t*, uint32_t) { }) { }

  // TODO presently these are not used 
  std::string access_token;
  std::string collector_host;
  uint32_t    collector_port;
  std::string collector_encryption;

  // The name of this LightStep component (a.k.a. "group name").
  // TODO should this be tags["lightstep:component_name"] instead?
  std::string component_name;

  // Runtime attributes
  std::unordered_map<std::string, std::string> tags;

  // Indicates whether to sample.
  std::function<bool(uint64_t)> should_sample;

  // A method that receives encoded lightstep::ReportRequest objects
  std::function<void(const uint8_t* payload, uint32_t length)> binary_transport;
};

struct StartSpanOptions {
  // operation_name may be empty (and set later via Span.SetOperationName)
  std::string operation_name;

  // parent may specify Span instance that caused the new (child) Span to be
  // created.
  //
  // If null, start a "root" span (i.e., start a new trace).
  Span parent;

  // start_time overrides the Span's start time, or implicitly becomes
  // time.Now() if StartTime.IsZero().
  TimeStamp start_time;

  // Tags may have zero or more entries; the restrictions on map
  // values are identical to those for Span.SetTag(). May be nil.
  //
  // If specified, the caller hands off ownership of Tags at
  // StartSpanWithOptions() invocation time.
  Dictionary tags;

  // Builder methods
  StartSpanOptions& SetOperationName(const std::string& operation_name) {
    this->operation_name = operation_name;
    return *this;
  }
  StartSpanOptions& SetParent(Span parent) {
    this->parent = parent;
    return *this;
  }
  StartSpanOptions& SetTag(const std::string& key,
			   const Value& value) {
    this->tags.insert(std::make_pair(key, value));
    return *this;
  }
  StartSpanOptions& SetStartTime(TimeStamp start_time) {
    this->start_time = start_time;
    return *this;
  }
};

struct FinishSpanOptions {
  // finish_time specifies the Span's finish time, otherwise
  // system_clock::now() will be used if FinishTime is the default.
  //
  // finish_time must resolve to a timestamp that's >= the Span's
  // start_time (per StartSpanOptions).
  TimeStamp finish_time;

  // TODO bulk_log_data
};

}  // namespace lightstep

#endif // __LIGHTSTEP_OPTIONS_H__
