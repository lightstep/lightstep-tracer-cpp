// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_IMPL_H__
#define __LIGHTSTEP_IMPL_H__

#include <chrono>
#include <memory>
#include <mutex>
#include <random>

#include "collector.pb.h"
#include "lightstep/options.h"
#include "lightstep/propagation.h"
#include "lightstep/value.h"

namespace lightstep {

typedef std::lock_guard<std::mutex> MutexLock;

class Recorder;
class SpanImpl;

typedef std::unordered_map<std::string, std::string> Baggage;

class ContextImpl {
public:
  ContextImpl() : trace_id(0), span_id(0) { }

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
  
  // These are modified during constructors (StartSpan and Extract),
  // but would require extra work/copying to make them const.
  uint64_t trace_id;
  uint64_t span_id;

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
  const std::string& access_token() const { return options_.access_token; }

  uint64_t reporter_id() const { return reporter_id_; }

  TimeStamp tracer_start_time() const { return tracer_start_time_; }
  const Attributes& tracer_attributes() const { return options_.tracer_attributes; }

  void RecordSpan(collector::Span&& span);

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

  bool inject(SpanContext sc, CarrierFormat format, const CarrierWriter &writer);
  bool inject_basic_carrier(SpanContext sc, const CarrierWriter& opaque);

  SpanContext extract(CarrierFormat format, const CarrierReader& reader);
  SpanContext extract_basic_carrier(const CarrierReader& reader);
  
  void GetTwoIds(uint64_t *a, uint64_t *b);
  uint64_t GetOneId();

  TracerOptions options_;
  uint64_t reporter_id_;
  std::shared_ptr<Recorder> recorder_;

  // Protects rand_source_, recorder_.
  std::mutex mutex_;

  // N.B. This may become a source of contention, if and when it does,
  // it may be converted to use a thread-local cache or thread-local
  // source.
  std::mt19937_64 rand_source_;

  // Either from TracerOptions.component_name or set by default logic.
  // TODO remove this, it's set in options_.attributes.
  std::string component_name_;

  // Start time of this process.
  // TODO remove this, it's no longer part of the protocol.
  TimeStamp tracer_start_time_;
};

class SpanImpl {
public:
  explicit SpanImpl(ImplPtr tracer)
    : tracer_(tracer) { }

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
  friend class Span;
  friend class SpanContext;
  friend class SpanReference;
  friend class SetTag;
  friend class StartTimestamp;
  friend class FinishTimestamp;
  friend class TracerImpl;

  // Fields set in StartSpan() are not protected by a mutex.
  ImplPtr       tracer_;
  TimeStamp     start_timestamp_;
  TimeStamp     finish_timestamp_;
  SpanReference ref_;
  ContextImpl   context_;

  // Mutex protects tags_ and operation_name_.
  std::mutex    mutex_;
  std::string   operation_name_;
  Dictionary    tags_;
};

} // namespace lightstep

#endif
