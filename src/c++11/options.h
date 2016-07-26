// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_OPTIONS_H__
#define __LIGHTSTEP_OPTIONS_H__

// Options for Tracer implementations, starting Spans, and finishing
// Spans.

#include <chrono>

#include "util.h"
#include "value.h"

namespace lightstep {

class SpanImpl;
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

// SpanStartOptions
class SpanStartOption {
public:
  virtual ~SpanStartOption() { }

  virtual void Apply(SpanImpl* span) const = 0;
};

class StartTimestamp : public SpanStartOption {
public:
  StartTimestamp(TimeStamp when) : when_(when) { }

  virtual void Apply(SpanImpl *span) const override;

private:
  TimeStamp when_;
};

class AddTag : public SpanStartOption {
public:
  AddTag(const std::string& key, const Value& value)
    : key_(key), value_(value) { }

  virtual void Apply(SpanImpl *span) const override;

private:
  const std::string &key_;
  const Value &value_;
};

// SpanFinishOption
class SpanFinishOption {
public:
  virtual ~SpanFinishOption() { }

  virtual void Apply(lightstep_net::SpanRecord *span) const = 0;
};

class FinishTimestampOption : public SpanFinishOption {
public:
  FinishTimestampOption(TimeStamp when) : when_(when) { }

  virtual void Apply(lightstep_net::SpanRecord *span) const override;

private:
  TimeStamp when_;
};

// This is unsafe to do. Need to consult with a C++11 stylist.
//
// This is like an unsafe std::reference_wrapper<> that allows taking
// references to temporaries.
template <typename T>
class option_wrapper {
public:
  option_wrapper(const T &opt) : ptr_(&opt) { }

  // This will dangle unless it is only used for short-lived initializer lists.
  const T& get() const { return *ptr_; }

private:
  const T *ptr_;
};

typedef std::initializer_list<option_wrapper<SpanStartOption>> SpanStartOptions;
typedef std::initializer_list<option_wrapper<SpanFinishOption>> SpanFinishOptions;

}  // namespace lightstep

#endif // __LIGHTSTEP_OPTIONS_H__
