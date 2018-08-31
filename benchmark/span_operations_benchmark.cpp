#include <benchmark/benchmark.h>
#include <lightstep/tracer.h>
#include <cassert>

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

static std::shared_ptr<opentracing::Tracer> MakeTracer() {
  lightstep::LightStepTracerOptions options;
  options.access_token = "abc123";
  options.transporter.reset(new NullTransporter{});
  return lightstep::MakeLightStepTracer(std::move(options));
}

static void BM_SpanCreation(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
  }
}
BENCHMARK(BM_SpanCreation);

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

static void BM_SpanSetTag1(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    span->SetTag("abc", "123");
  }
}
BENCHMARK(BM_SpanSetTag1);

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

static void BM_SpanLog1(benchmark::State& state) {
  auto tracer = MakeTracer();
  assert(tracer != nullptr);
  for (auto _ : state) {
    auto span = tracer->StartSpan("abc123");
    span->Log({{"abc", 123}});
  }
}
BENCHMARK(BM_SpanLog1);

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

BENCHMARK_MAIN();
