#include <benchmark/benchmark.h>
#include <lightstep/tracer.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <thread>

const int num_spans_for_span_creation_threaded_benchmark = 2000;

//------------------------------------------------------------------------------
// NullSyncTransporter
//------------------------------------------------------------------------------
namespace {
class NullSyncTransporter final : public lightstep::SyncTransporter {
 public:
  opentracing::expected<void> Send(
      const google::protobuf::Message& /*request*/,
      google::protobuf::Message& /*response*/) override {
    return {};
  }
};
}  // namespace

//------------------------------------------------------------------------------
// NullStreamTransporter
//------------------------------------------------------------------------------
namespace {
class NullStreamTransporter final : public lightstep::StreamTransporter {
 public:
  size_t Write(const char* /*buffer*/, size_t size) override { return size; }
};
}  // namespace

//------------------------------------------------------------------------------
// MakeRpcTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeRpcTracer() {
  lightstep::LightStepTracerOptions options;
  options.access_token = "abc123";
  options.collector_plaintext = true;
  options.transporter.reset(new NullSyncTransporter{});
  return lightstep::MakeLightStepTracer(std::move(options));
}

//------------------------------------------------------------------------------
// MakeStreamTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeStreamTracer() {
  lightstep::LightStepTracerOptions options;
  options.access_token = "abc123";
  options.collector_plaintext = true;
  options.use_stream_recorder = true;
  options.transporter.reset(new NullStreamTransporter{});
  return lightstep::MakeLightStepTracer(std::move(options));
}

//------------------------------------------------------------------------------
// MakeTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeTracer(
    opentracing::string_view tracer_type) {
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

//------------------------------------------------------------------------------
// BM_SpanCreation
//------------------------------------------------------------------------------
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
// BM_SpanCreationWithParent
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// BM_SpanCreationThreaded
//------------------------------------------------------------------------------
static void BM_SpanCreationThreaded(benchmark::State& state,
                                    const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  auto num_threads = state.range(0);
  auto n = static_cast<int>(num_spans_for_span_creation_threaded_benchmark);
  auto num_spans_per_thread = n / num_threads;
  auto remainder = n - num_spans_per_thread * num_threads;
  for (auto _ : state) {
    std::vector<std::thread> threads(num_threads);
    for (int i = 0; i < num_threads; ++i) {
      auto num_spans_for_this_thread = num_spans_per_thread + (i < remainder);
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

//------------------------------------------------------------------------------
// BM_SpanSetTag1
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// BM_SpanSetTag2
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// BM_SpanLog1
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// BM_SpanLog2
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
BENCHMARK_MAIN();
