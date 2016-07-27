// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_IMPL_H__
#define __LIGHTSTEP_IMPL_H__

// TODO some comments, please

#include <chrono>
#include <memory>
#include <mutex>
#include <random>

#include "options.h"
#include "propagation.h"
#include "value.h"

namespace lightstep_net {
class SpanRecord;
} // namespace lightstep_net

namespace lightstep {

typedef std::lock_guard<std::mutex> MutexLock;

class Recorder;
class SpanImpl;

typedef std::unordered_map<std::string, std::string> Baggage;

class ContextImpl {
public:
  ContextImpl() : trace_id(0), span_id(0), parent_span_id(0), sampled(false) { }

  void setBaggageItem(std::pair<std::string, std::string>&& item) {
    MutexLock l(baggage_mutex_);
    baggage_.emplace(item);
  }

  std::string baggageItem(const std::string& key) {
    MutexLock l(baggage_mutex_);
    auto lookup = baggage_.find(key);
    if (lookup != baggage_.end()) {
      return lookup->second;
    }
    return "";
  }

  void foreachBaggageItem(std::function<bool(const std::string& key,
					     const std::string& value)> f) const {
    MutexLock l(baggage_mutex_);
    for (const auto& bi : baggage_) {
      if (!f(bi.first, bi.second)) {
	return;
      }
    }
  }
  
  // These four fields are only modified in StartSpan. TODO could
  // rearrange that code to make these fields const.
  uint64_t trace_id;
  uint64_t span_id;
  uint64_t parent_span_id;
  bool     sampled;

private:
  mutable std::mutex baggage_mutex_;
  Baggage baggage_;
};

class TracerImpl {
 public:
  explicit TracerImpl(const TracerOptions& options);

  std::unique_ptr<SpanImpl> StartSpan(std::shared_ptr<TracerImpl> selfptr,
				      const std::string& op_name,
				      SpanStartOptions opts = {});
  
  const TracerOptions& options() const { return options_; }

  const std::string& component_name() const { return component_name_; }
  const std::string& runtime_guid() const { return runtime_guid_; }
  const std::string& access_token() const { return options_.access_token; }
  uint64_t           runtime_start_micros() const { return runtime_micros_; }
  const Attributes&  runtime_attributes() const { return options_.runtime_attributes; }

  void RecordSpan(lightstep_net::SpanRecord&& span);

  void Flush();

  void Stop() {
    std::shared_ptr<Recorder> recorder;
    {
      MutexLock lock(mutex_);
      std::swap(recorder_, recorder);
    }
    recorder.reset();
  }

  std::shared_ptr<Recorder> recorder() {
    MutexLock lock(mutex_);
    return recorder_;
  }

  void set_recorder(std::unique_ptr<Recorder> recorder);

 private:
  friend class Tracer;
  friend class SpanReference;

  bool inject(SpanContext sc, const CarrierFormat &format, const CarrierWriter &writer);
  SpanContext extract(const CarrierFormat& format, const CarrierReader& reader);
  
  void GetTwoIds(uint64_t *a, uint64_t *b);
  uint64_t GetOneId();

  TracerOptions options_;
  std::shared_ptr<Recorder> recorder_;

  // Protects rand_source_, recorder_.
  std::mutex mutex_;

  // N.B. This may become a source of contention, if and when it does,
  // it may be converted to use a thread-local cache or thread-local
  // source.
  std::mt19937_64 rand_source_;

  // Computed from rand_source_, hexified.
  std::string runtime_guid_;

  // Either from TracerOptions.component_name or set by default logic.
  std::string component_name_;

  // Start time of this runtime process, in microseconds.
  uint64_t runtime_micros_;
};

class SpanImpl {
public:
  explicit SpanImpl(ImplPtr tracer)
    : start_micros_(0), tracer_(tracer) { }

  void SetOperationName(const std::string& name) {
    MutexLock l(mutex_);
    operation_name_ = name;
  }

  void SetTag(const std::string& key, const Value& value) {
    MutexLock l(mutex_);
    tags_[key] = value;
  }

  void SetBaggageItem(const std::string& key,
		      const std::string& value) {
    context_.setBaggageItem(std::make_pair(key, value));
  }

  std::string BaggageItem(const std::string& key) {
    return context_.baggageItem(key);
  }

  void FinishSpan(SpanFinishOptions opts = {});

private:
  friend class AddTag;
  friend class Span;
  friend class SpanContext;
  friend class SpanReference;
  friend class StartTimestamp;
  friend class TracerImpl;

  ImplPtr     tracer_;
  std::mutex  mutex_;  // Protects Baggage and Tags.
  std::string operation_name_;
  uint64_t    start_micros_;
  ContextImpl context_;
  Dictionary  tags_;
};

} // namespace lightstep

#endif
