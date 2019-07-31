#include "span.h"

#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <thread>
#include <vector>

#include "test/recorder/in_memory_recorder.h"
#include "tracer/legacy/legacy_tracer_impl.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeBenchmarkSpan
//--------------------------------------------------------------------------------------------------
static std::unique_ptr<opentracing::Span> MakeBenchmarkSpan(
    opentracing::Tracer& tracer, const char* payload) {
  static std::atomic<uint32_t> SpanCounter{0};
  auto id = SpanCounter++;
  std::array<char, 8 + 1> operation_name;
  auto operation_name_length =
      std::snprintf(operation_name.data(), operation_name.size(), "%08X", id);
  assert(operation_name_length > 0);
  auto span = tracer.StartSpan(opentracing::string_view{
      operation_name.data(), static_cast<size_t>(operation_name_length)});
  if (payload != nullptr) {
    span->SetTag("payload", payload);
  }
  return span;
}

//------------------------------------------------------------------------------
// GenerateSpansForThread
//------------------------------------------------------------------------------
static void GenerateSpansForThread(opentracing::Tracer& tracer, int num_spans,
                                   std::chrono::nanoseconds min_span_elapse,
                                   const char* payload) {
  auto start_timestamp = std::chrono::system_clock::now();
  for (int i = 0; i < num_spans; ++i) {
    MakeBenchmarkSpan(tracer, payload);
    std::this_thread::sleep_until(
        start_timestamp +
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            min_span_elapse * i));
  }
}

//--------------------------------------------------------------------------------------------------
// GenerateSpans
//--------------------------------------------------------------------------------------------------
void GenerateSpans(opentracing::Tracer& tracer,
                   const tracer_upload_bench::Configuration& config) {
  auto seconds_per_span = 1.0 / config.spans_per_second();
  auto min_span_elapse =
      std::chrono::nanoseconds{static_cast<size_t>(seconds_per_span * 1.0e9)};
  std::string payload;
  const char* payload_ptr = nullptr;
  if (config.payload_size() > 0) {
    payload = std::string(config.payload_size(), 'X');
    payload_ptr = payload.data();
  }
  std::vector<std::thread> threads(config.num_threads());
  for (auto& thread : threads) {
    thread = std::thread{GenerateSpansForThread, std::ref(tracer),
                         config.num_spans_per_thread(), min_span_elapse,
                         payload_ptr};
  };
  for (auto& thread : threads) {
    thread.join();
  }
  tracer.Close();
}

//--------------------------------------------------------------------------------------------------
// ComputeSpanSize
//--------------------------------------------------------------------------------------------------
size_t ComputeSpanSize(const tracer_upload_bench::Configuration& config) {
  auto recorder = new InMemoryRecorder{};
  auto tracer = std::shared_ptr<opentracing::Tracer>{new LegacyTracerImpl{
      PropagationOptions{}, std::unique_ptr<Recorder>{recorder}}};
  std::string payload;
  if (config.payload_size() > 0) {
    payload = std::string(config.payload_size(), 'X');
  }
  if (payload.empty()) {
    MakeBenchmarkSpan(*tracer, nullptr);
  } else {
    MakeBenchmarkSpan(*tracer, payload.data());
  }
  tracer->Close();
  return recorder->top().ByteSizeLong();
}
}  // namespace lightstep
