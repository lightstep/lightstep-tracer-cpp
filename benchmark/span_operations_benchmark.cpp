#include <cassert>

#include "lightstep/tracer.h"

#include "recorder/stream_recorder/stream_recorder.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "benchmark/benchmark.h"

//--------------------------------------------------------------------------------------------------
// NullTransport
//--------------------------------------------------------------------------------------------------
namespace {
class NullTransporter final : public lightstep::SyncTransporter {
 public:
  opentracing::expected<void> Send(
      const google::protobuf::Message& /*request*/,
      google::protobuf::Message& /*response*/) override {
    return {};
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// NullTextMapWriter
//--------------------------------------------------------------------------------------------------
namespace {
class NullTextMapWriter final : public opentracing::TextMapWriter {
 public:
  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override {
    benchmark::DoNotOptimize(key.data());
    benchmark::DoNotOptimize(value.data());
    return {};
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// TextMapWriter
//--------------------------------------------------------------------------------------------------
namespace {
class TextMapWriter final : public opentracing::TextMapWriter {
 public:
  explicit TextMapWriter(
      std::vector<std::pair<std::string, std::string>>& key_values)
      : key_values_{key_values} {}

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override {
    key_values_.emplace_back(key, value);
    return {};
  }

 private:
  std::vector<std::pair<std::string, std::string>>& key_values_;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// TextMapReader
//--------------------------------------------------------------------------------------------------
namespace {
class TextMapReader final : public opentracing::TextMapReader {
 public:
  explicit TextMapReader(
      const std::vector<std::pair<std::string, std::string>>& key_values)
      : key_values_{key_values} {}

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override {
    for (auto& key_value : key_values_) {
      auto was_successful = f(key_value.first, key_value.second);
      if (!was_successful) {
        return was_successful;
      }
    }
    return {};
  }

 private:
  const std::vector<std::pair<std::string, std::string>>& key_values_;
};
}  // namespace
//
//------------------------------------------------------------------------------
// MakeRpcTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeRpcTracer() {
  lightstep::LightStepTracerOptions options;
  options.access_token = "abc123";
  options.transporter.reset(new NullTransporter{});
  return lightstep::MakeLightStepTracer(std::move(options));
}

//--------------------------------------------------------------------------------------------------
// MakeStreamTracer
//--------------------------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeStreamTracer() {
  lightstep::LightStepTracerOptions tracer_options;
  lightstep::StreamRecorderOptions recorder_options;
  recorder_options.throw_away_spans = true;
  auto logger = std::make_shared<lightstep::Logger>();
  logger->set_level(lightstep::LogLevel::off);
  auto stream_recorder = new lightstep::StreamRecorder{
      *logger, std::move(tracer_options), std::move(recorder_options)};
  std::unique_ptr<lightstep::Recorder> recorder{stream_recorder};
  lightstep::PropagationOptions propagation_options;

  return std::make_shared<lightstep::TracerImpl>(
      logger,
      lightstep::MakePropagationOptions(lightstep::LightStepTracerOptions{}),
      std::move(recorder));
}

//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeTracer(
    opentracing::string_view tracer_type = "rpc") {
  if (tracer_type == "rpc") {
    return MakeRpcTracer();
  }
  if (tracer_type == "stream") {
    return MakeStreamTracer();
  }
  std::cerr << "Unknown tracer type: " << tracer_type << "\n";
  std::terminate();
}

//------------------------------------------------------------------------------
// MakeSpans
//------------------------------------------------------------------------------
static void MakeSpans(const opentracing::Tracer& tracer, int n) {
  for (int i = 0; i < n; ++i) {
    auto span = tracer.StartSpan("abc123");
  }
}

//--------------------------------------------------------------------------------------------------
// BM_SpanCreation
//--------------------------------------------------------------------------------------------------
static void BM_SpanCreation(benchmark::State& state, const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
  }
}
BENCHMARK_CAPTURE(BM_SpanCreation, rpc, "rpc");
BENCHMARK_CAPTURE(BM_SpanCreation, stream, "stream");

//------------------------------------------------------------------------------
// BM_SpanCreationThreaded
//------------------------------------------------------------------------------
static void BM_SpanCreationThreaded(benchmark::State& state,
                                    const char* tracer_type) {
  const int num_spans_for_span_creation_threaded_benchmark = 4000;
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  auto num_threads = state.range(0);
  auto n = static_cast<int>(num_spans_for_span_creation_threaded_benchmark);
  auto num_spans_per_thread = n / num_threads;
  auto remainder = n - num_spans_per_thread * num_threads;
  for (auto _ : state) {
    std::vector<std::thread> threads(num_threads);
    for (int i = 0; i < num_threads; ++i) {
      auto num_spans_for_this_thread =
          num_spans_per_thread + static_cast<int>(i < remainder);
      threads[i] =
          std::thread{&MakeSpans, std::ref(*tracer), num_spans_for_this_thread};
    }
    for (auto& thread : threads) {
      thread.join();
    }
  }
}
BENCHMARK_CAPTURE(BM_SpanCreationThreaded, rpc, "rpc")
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8);
BENCHMARK_CAPTURE(BM_SpanCreationThreaded, stream, "stream")
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8);

//--------------------------------------------------------------------------------------------------
// BM_SpanCreationWithParent
//--------------------------------------------------------------------------------------------------
static void BM_SpanCreationWithParent(benchmark::State& state,
                                      const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  auto parent_span = tracer->StartSpan("parent");
  parent_span->Finish();
  opentracing::StartSpanOptions options;
  options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                  &parent_span->context());
  for (auto _ : state) {
    auto span = tracer->StartSpanWithOptions("abc123", options);
  }
}
BENCHMARK_CAPTURE(BM_SpanCreationWithParent, rpc, "rpc");
BENCHMARK_CAPTURE(BM_SpanCreationWithParent, stream, "stream");

//--------------------------------------------------------------------------------------------------
// BM_SpanSetTag1
//--------------------------------------------------------------------------------------------------
static void BM_SpanSetTag1(benchmark::State& state, const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    span->SetTag("abc", "123");
  }
}
BENCHMARK_CAPTURE(BM_SpanSetTag1, rpc, "rpc");
BENCHMARK_CAPTURE(BM_SpanSetTag1, stream, "stream");

//--------------------------------------------------------------------------------------------------
// BM_SpanSetTag2
//--------------------------------------------------------------------------------------------------
static void BM_SpanSetTag2(benchmark::State& state, const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    char key[5];
    key[0] = 'a';
    key[1] = 'b';
    key[2] = 'c';
    key[3] = '0';
    key[4] = '\0';
    for (int i = 0; i < 10; ++i) {
      span->SetTag(opentracing::string_view{key, 4}, "123");
      ++key[3];
    }
  }
}
BENCHMARK_CAPTURE(BM_SpanSetTag2, rpc, "rpc");
BENCHMARK_CAPTURE(BM_SpanSetTag2, stream, "stream");

//--------------------------------------------------------------------------------------------------
// BM_SpanLog1
//--------------------------------------------------------------------------------------------------
static void BM_SpanLog1(benchmark::State& state, const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    span->Log({{"abc", 123}});
  }
}
BENCHMARK_CAPTURE(BM_SpanLog1, rpc, "rpc");
BENCHMARK_CAPTURE(BM_SpanLog1, stream, "stream");

//--------------------------------------------------------------------------------------------------
// BM_SpanLog2
//--------------------------------------------------------------------------------------------------
static void BM_SpanLog2(benchmark::State& state, const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    for (int i = 0; i < 10; ++i) {
      span->Log({{"abc", 123}});
    }
  }
}
BENCHMARK_CAPTURE(BM_SpanLog2, rpc, "rpc");
BENCHMARK_CAPTURE(BM_SpanLog2, stream, "stream");

//--------------------------------------------------------------------------------------------------
// BM_SpanContextMultikeyInjection
//--------------------------------------------------------------------------------------------------
static void BM_SpanContextMultikeyInjection(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  auto span = tracer->StartSpan("abc123");
  assert(span != nullptr);
  NullTextMapWriter carrier;
  for (auto _ : state) {
    auto was_successful = tracer->Inject(span->context(), carrier);
    assert(was_successful);
  }
}
BENCHMARK(BM_SpanContextMultikeyInjection);

//--------------------------------------------------------------------------------------------------
// BM_SpanContextMultikeyExtraction
//--------------------------------------------------------------------------------------------------
static void BM_SpanContextMultikeyExtraction(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  auto span = tracer->StartSpan("abc123");
  std::vector<std::pair<std::string, std::string>> key_values;
  auto was_successful =
      tracer->Inject(span->context(), TextMapWriter{key_values});
  assert(was_successful);
  TextMapReader carrier{key_values};
  for (auto _ : state) {
    auto span_context_maybe = tracer->Extract(carrier);
    assert(span_context_maybe);
    benchmark::DoNotOptimize(span_context_maybe);
  }
}
BENCHMARK(BM_SpanContextMultikeyExtraction);

//--------------------------------------------------------------------------------------------------
// BM_SpanContextMultikeyEmptyExtraction
//--------------------------------------------------------------------------------------------------
static void BM_SpanContextMultikeyEmptyExtraction(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  std::vector<std::pair<std::string, std::string>> key_values = {
      {"Accept", "text/html"},
      {"Content-Length", "300"},
      {"User-Agent", "Mozilla/5.0"},
      {"Pragma", "no-cache"},
      {"Host", "www.memyselfandi.com"}};
  TextMapReader carrier{key_values};
  for (auto _ : state) {
    auto span_context_maybe = tracer->Extract(carrier);
    assert(span_context_maybe);
    assert(*span_context_maybe == nullptr);
    benchmark::DoNotOptimize(span_context_maybe);
  }
}
BENCHMARK(BM_SpanContextMultikeyEmptyExtraction);

//--------------------------------------------------------------------------------------------------
// BENCHMARK_MAIN
//--------------------------------------------------------------------------------------------------
BENCHMARK_MAIN();
