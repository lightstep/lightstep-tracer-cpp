#include "span_generator.h"
#include <cassert>
#include <random>
#include <string>

using namespace opentracing;

static thread_local std::mt19937 rng{std::random_device()()};

const int num_threads = 5;

namespace lightstep {
//------------------------------------------------------------------------------
// GenerateRandomDuration
//------------------------------------------------------------------------------
static std::chrono::steady_clock::duration GenerateRandomDuration(
    const std::chrono::steady_clock::duration& max_duration) {
  auto t = std::uniform_real_distribution<double>(0, 1)(rng);
  return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      t * max_duration);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
SpanGenerator::SpanGenerator(
    const Tracer& tracer,
    const std::chrono::steady_clock::duration& avg_time_between_spans)
    : tracer_{tracer},
      max_span_duration_{avg_time_between_spans / 10},
      max_time_between_spans_{2 * avg_time_between_spans} {
  for (int i = 0; i < num_threads; ++i) {
    generator_threads_.emplace_back(
        std::thread(&SpanGenerator::GenerateSpans, this));
  }
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
SpanGenerator::~SpanGenerator() {
  std::unique_lock<std::mutex> lock_guard{mutex_};
  exit_ = true;
  lock_guard.unlock();
  condition_variable_.notify_all();
  for (auto& thread : generator_threads_) thread.join();
}

//------------------------------------------------------------------------------
// Run
//------------------------------------------------------------------------------
void SpanGenerator::Run(std::chrono::steady_clock::duration duration) {
  // Reduce the requested duration so as to ensure that any spans started
  // get finish within the `duration` time window.
  duration -= 2 * max_span_duration_;

  auto start_time = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start_time < duration) {
    auto wait_time = GenerateRandomDuration(max_time_between_spans_);
    std::this_thread::sleep_for(wait_time);
    std::unique_lock<std::mutex> lock_guard{mutex_};
    ++span_generation_counter_;
    lock_guard.unlock();
    condition_variable_.notify_one();
  }

  std::this_thread::sleep_for(2 * max_span_duration_);
}

//------------------------------------------------------------------------------
// GenerateSpans
//------------------------------------------------------------------------------
void SpanGenerator::GenerateSpans() {
  while (1) {
    std::unique_lock<std::mutex> lock_guard{mutex_};
    condition_variable_.wait(
        lock_guard, [this] { return span_generation_counter_ > 0 || exit_; });
    if (exit_) return;
    --span_generation_counter_;
    lock_guard.unlock();

    auto span = tracer_.StartSpan(std::to_string(rng()));
    assert(span);
    auto wait_time = GenerateRandomDuration(max_span_duration_);
    std::this_thread::sleep_for(wait_time);
    span->Finish();
    ++num_spans_generated_;
  }
}
}  // namespace lightstep
