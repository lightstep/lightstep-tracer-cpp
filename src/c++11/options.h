// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_OPTIONS_H__
#define __LIGHTSTEP_OPTIONS_H__

// Options for Tracer implementations, starting Spans, and finishing
// Spans.

#include <chrono>

#include "span.h"
#include "util.h"
#include "value.h"

namespace lightstep {

class TracerImpl;

typedef std::unordered_map<std::string, std::string> Attributes;
typedef std::shared_ptr<TracerImpl> ImplPtr;

class TracerOptions {
public:
  std::string access_token;
  std::string collector_host;
  uint32_t    collector_port;
  std::string collector_encryption;

  // Runtime attributes
  Attributes runtime_attributes;

  // Indicates whether to collect this span.
  std::function<bool(uint64_t)> should_sample;
};

// StartSpanOptions
class StartSpanOption {
public:
  virtual ~StartSpanOption() { }

  virtual void Apply(SpanImpl* span) const = 0;
};

class StartTimestampOption : public StartSpanOption {
public:
  StartTimestampOption(TimeStamp when) : when_(when) { }

  virtual void Apply(SpanImpl *span) const;

private:
  TimeStamp when_;
};

class StartTagOption : public StartSpanOption {
public:
  StartTagOption(const std::string& key, const Value& value)
    : key_(key), value_(value) { }

  virtual void Apply(SpanImpl *span) const;

private:
  const std::string &key_;
  const Value &value_;
};

// FinishSpanOptions
class FinishSpanOption {
public:
  virtual ~FinishSpanOption() { }

  virtual void Apply(lightstep_net::SpanRecord *span) const = 0;
};

class FinishTimestampOption : public FinishSpanOption {
public:
  FinishTimestampOption(TimeStamp when) : when_(when) { }

  virtual void Apply(lightstep_net::SpanRecord *span) const;

private:
  TimeStamp when_;
};

}  // namespace lightstep

#endif // __LIGHTSTEP_OPTIONS_H__
