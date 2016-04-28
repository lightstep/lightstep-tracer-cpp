// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_IMPL_H__
#define __LIGHTSTEP_IMPL_H__

#include <chrono>
#include <memory>
#include <mutex>
#include <random>

#include "options.h"
#include "value.h"

namespace lightstep {

typedef std::lock_guard<std::mutex> MutexLock;

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

 private:
  friend class SpanImpl;

  void GetTwoIds(uint64_t *a, uint64_t *b);
  uint64_t GetOneId();

  const TracerOptions options_;

  // Protects rand_source_
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
  std::vector<lightstep_thrift::KeyValue> runtime_attributes_;
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

namespace util {

std::string id_to_string(uint64_t);
std::string program_name();

uint64_t to_micros(TimeStamp t);

} // namespace util 
} // namespace lightstep

#endif
