// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_IMPL_H__
#define __LIGHTSTEP_IMPL_H__

// TODO some comments, please

#include <chrono>
#include <memory>
#include <mutex>
#include <random>

#include "options.h"
#include "value.h"

namespace lightstep_net {
class SpanRecord;
} // namespace lightstep_net

namespace lightstep {

typedef std::lock_guard<std::mutex> MutexLock;

class Recorder;
class SpanImpl;
struct TracerOptions;
struct StartSpanOptions;
struct FinishSpanOptions;

typedef std::unordered_map<std::string, std::string> Baggage;

struct Context {
  uint64_t trace_id;
  uint64_t span_id;
  uint64_t parent_span_id;
  bool     sampled;
  Baggage  baggage;
};

class TracerImpl {
 public:
  explicit TracerImpl(const TracerOptions& options);

  std::unique_ptr<SpanImpl> StartSpan(std::shared_ptr<TracerImpl> selfptr,
				      const StartSpanOptions& options);

  const TracerOptions& options() const { return options_; }

  const std::string& component_name() const { return component_name_; }
  const std::string& runtime_guid() const { return runtime_guid_; }
  const std::string& access_token() const { return options_.access_token; }
  uint64_t           runtime_start_micros() const { return runtime_micros_; }
  const Attributes&  runtime_attributes() const { return options_.tags; }

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

  // Runtime attributes.
  std::vector<std::pair<std::string, std::string>> runtime_attributes_;
};

class SpanImpl {
public:
  void SetOperationName(const std::string& name) {
    MutexLock l(mutex_);
    operation_name_ = name;
  }

  void SetTag(const std::string& key, const Value& value) {
    MutexLock l(mutex_);
    tags_[key] = value;

  }

  void SetBaggageItem(const std::string& restricted_key,
		      const std::string& value) {
    MutexLock l(mutex_);
    context_.baggage.insert(std::make_pair(restricted_key, value));
  }

  std::string BaggageItem(const std::string& restricted_key) {
    MutexLock l(mutex_);
    auto lookup = context_.baggage.find(restricted_key);
    if (lookup != context_.baggage.end()) {
      return lookup->second;
    }
    return "";
  }

  void FinishSpan(const FinishSpanOptions& options);

private:
  friend class TracerImpl;
  friend class Span;

  // Protects Baggage and Tags.
  std::mutex  mutex_;
  std::string operation_name_;
  uint64_t    start_micros_;
  Context     context_;
  Dictionary  tags_;
  std::shared_ptr<TracerImpl> tracer_;
};

} // namespace lightstep

#endif
