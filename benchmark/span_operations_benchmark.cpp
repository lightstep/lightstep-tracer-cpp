#include <benchmark/benchmark.h>
#include <lightstep/tracer.h>

#include <cassert>
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
// MakeTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> MakeTracer() {
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
static void BM_SpanCreation(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
  }
}
BENCHMARK(BM_SpanCreation);

//------------------------------------------------------------------------------
// BM_StreamSpanCreation
//------------------------------------------------------------------------------
static void BM_StreamSpanCreation(benchmark::State& state) {
  auto tracer = MakeStreamTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
  }
}
BENCHMARK(BM_StreamSpanCreation);

//------------------------------------------------------------------------------
// BM_SpanCreationWithParent
//------------------------------------------------------------------------------
static void BM_SpanCreationWithParent(benchmark::State& state) {
  auto tracer = MakeTracer();
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
BENCHMARK(BM_SpanCreationWithParent);

//------------------------------------------------------------------------------
// BM_SpanCreationThreaded
//------------------------------------------------------------------------------
static void BM_SpanCreationThreaded(benchmark::State& state) {
  auto tracer = MakeTracer();
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
BENCHMARK(BM_SpanCreationThreaded)->Arg(2)->Arg(4)->Arg(8);

//------------------------------------------------------------------------------
// BM_StreamSpanCreationThreaded
//------------------------------------------------------------------------------
static void BM_StreamSpanCreationThreaded(benchmark::State& state) {
  auto tracer = MakeStreamTracer();
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
BENCHMARK(BM_StreamSpanCreationThreaded)->Arg(2)->Arg(4)->Arg(8);

//------------------------------------------------------------------------------
// BM_SpanSetTag1
//------------------------------------------------------------------------------
static void BM_SpanSetTag1(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    span->SetTag("abc", "123");
  }
}
BENCHMARK(BM_SpanSetTag1);

//------------------------------------------------------------------------------
// BM_SpanSetTag2
//------------------------------------------------------------------------------
static void BM_SpanSetTag2(benchmark::State& state) {
  auto tracer = MakeTracer();
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
BENCHMARK(BM_SpanSetTag2);

//------------------------------------------------------------------------------
// BM_SpanLog1
//------------------------------------------------------------------------------
static void BM_SpanLog1(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    span->Log({{"abc", 123}});
  }
}
BENCHMARK(BM_SpanLog1);

//------------------------------------------------------------------------------
// BM_SpanLog2
//------------------------------------------------------------------------------
static void BM_SpanLog2(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    for (int i = 0; i < 10; ++i) {
      span->Log({{"abc", 123}});
    }
  }
}
BENCHMARK(BM_SpanLog2);

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
BENCHMARK_MAIN();
