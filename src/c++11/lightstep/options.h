// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_OPTIONS_H__
#define __LIGHTSTEP_OPTIONS_H__

// Options for Tracer implementations, starting Spans, and finishing
// Spans.

#include <chrono>
#include <memory>

#include "lightstep/util.h"
#include "lightstep/value.h"

namespace lightstep {

class SpanImpl;
class TracerImpl;

typedef std::unordered_map<std::string, std::string> Attributes;
typedef std::shared_ptr<TracerImpl> ImplPtr;

// TracerIDKey can be used as the key in TracerOptions.tracer_attributes
// to encode the LightStep tracer ID.
extern const char TracerIDKey[];

class TracerOptions {
public:
  // Required (for all recorders)
  std::string access_token;

  // Common fields (must be set to use default recorder)
  std::string collector_host;
  uint32_t    collector_port;
  std::string collector_encryption;

  // Tracer attributes.  To define the LightStep Tracer ID manually
  // set TracerIDKey here.
  Attributes tracer_attributes;
};

class SpanStartOption {
public:
  SpanStartOption(const SpanStartOption&) = delete;

  virtual ~SpanStartOption() { }

  virtual void Apply(SpanImpl* span) const = 0;

protected:
  SpanStartOption() { }
};

class StartTimestamp : public SpanStartOption {
public:
  StartTimestamp(TimeStamp when) : when_(when) { }
  StartTimestamp(const StartTimestamp& o) : when_(o.when_) { }

  virtual void Apply(SpanImpl *span) const override;

private:
  TimeStamp when_;
};

class SetTag : public SpanStartOption {
public:
  SetTag(const std::string& key, const Value& value)
    : key_(key), value_(value) { }
  SetTag(const SetTag& o) : key_(o.key_), value_(o.value_) { }

  virtual void Apply(SpanImpl *span) const override;

private:
  const std::string &key_;
  const Value &value_;
};

class SpanFinishOption {
public:
  SpanFinishOption(const SpanFinishOption&) = delete;
  virtual ~SpanFinishOption() { }

  virtual void Apply(SpanImpl *span) const = 0;

protected:
  SpanFinishOption() { };
};

class FinishTimestamp : public SpanFinishOption {
public:
  FinishTimestamp(TimeStamp when) : when_(when) { }
  FinishTimestamp(const FinishTimestamp &o) : when_(o.when_) { }

  virtual void Apply(SpanImpl *span) const override;

private:
  TimeStamp when_;
};

// This is unsafe to do.
//
// This is like an unsafe std::reference_wrapper<> that allows taking
// references to temporaries.  It must only be used for temporary
// SpanStartOption and SpanFinishOption objects.
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
