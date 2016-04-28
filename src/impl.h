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

  const TracerOptions options_;

  // Protects rand_source_
  std::mutex mutex_;

  // N.B. This may become a source of contention, if and when it does,
  // it may be converted to use a thread-local cache or thread-local
  // source.
  std::mt19937_64 rand_source_;
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

  // Protects Baggage and Tags.
  std::mutex  mutex_;
  std::string operation_name_;
  TimeStamp   start_time_;
  Context     context_;
  Dictionary  tags_;
  std::shared_ptr<TracerImpl> tracer_;
};

} // namespace lightstep

#endif
