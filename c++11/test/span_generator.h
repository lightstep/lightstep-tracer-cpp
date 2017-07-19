#pragma once

#include <opentracing/tracer.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace lightstep {
class SpanGenerator {
 public:
  SpanGenerator(
      const opentracing::Tracer& tracer,
      const std::chrono::steady_clock::duration& avg_time_between_spans);

  ~SpanGenerator();

  void Run(std::chrono::steady_clock::duration duration);

  int num_spans_generated() const { return num_spans_generated_; }

 private:
  const opentracing::Tracer& tracer_;
  std::chrono::steady_clock::duration max_span_duration_;
  std::chrono::steady_clock::duration max_time_between_spans_;
  std::vector<std::thread> generator_threads_;

  std::mutex mutex_;
  std::condition_variable condition_variable_;
  int span_generation_counter_ = 0;
  std::atomic<int> num_spans_generated_{0};
  bool exit_ = false;

  void GenerateSpans();
};
}  // namespace lightstep
