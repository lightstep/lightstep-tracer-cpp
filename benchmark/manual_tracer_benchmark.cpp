#include <cassert>
#include <memory>

#include "lightstep/tracer.h"

#include "benchmark/benchmark.h"

int MaxBufferedSpans = 500;

//--------------------------------------------------------------------------------------------------
// LegacyNullTransport
//--------------------------------------------------------------------------------------------------
namespace {
class LegacyNullTransporter final : public lightstep::LegacyAsyncTransporter {
 public:
  // LegacyAsyncTransporter
  void Send(const google::protobuf::Message& request,
            google::protobuf::Message& /*response*/,
            Callback& callback) override {
    using google::protobuf::uint8;
    auto size = request.ByteSizeLong();
    std::unique_ptr<uint8> buffer{new uint8[size]};
    request.SerializeWithCachedSizesToArray(buffer.get());
    callback.OnSuccess();
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// NullTransporter
//--------------------------------------------------------------------------------------------------
namespace {
class NullTransporter final : public lightstep::AsyncTransporter {
 public:
  // AsyncTransporter
  void Send(std::unique_ptr<lightstep::BufferChain>&& message,
            Callback& callback) noexcept override {
    auto size = message->num_bytes();
    std::unique_ptr<char> buffer{new char[size]};
    message->CopyOut(buffer.get(), size);
    callback.OnSuccess(*message);
  }
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// MakeLegacyManualTracer
//--------------------------------------------------------------------------------------------------
static std::shared_ptr<lightstep::LightStepTracer> MakeLegacyManualTracer() {
  lightstep::LightStepTracerOptions tracer_options;
  tracer_options.transporter.reset(new LegacyNullTransporter{});
  tracer_options.use_thread = false;
  tracer_options.component_name = "abc";
  tracer_options.access_token = "123";
  tracer_options.max_buffered_spans = static_cast<size_t>(MaxBufferedSpans);

  return lightstep::MakeLightStepTracer(std::move(tracer_options));
}

//--------------------------------------------------------------------------------------------------
// MakeManualTracer
//--------------------------------------------------------------------------------------------------
static std::shared_ptr<lightstep::LightStepTracer> MakeManualTracer() {
  lightstep::LightStepTracerOptions tracer_options;
  tracer_options.transporter.reset(new NullTransporter{});
  tracer_options.use_thread = false;
  tracer_options.component_name = "abc";
  tracer_options.access_token = "123";
  tracer_options.max_buffered_spans = static_cast<size_t>(MaxBufferedSpans);

  return lightstep::MakeLightStepTracer(std::move(tracer_options));
}

//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
static std::shared_ptr<lightstep::LightStepTracer> MakeTracer(
    opentracing::string_view tracer_type = "legacy_manual") {
  if (tracer_type == "legacy_manual") {
    return MakeLegacyManualTracer();
  }
  if (tracer_type == "manual") {
    return MakeManualTracer();
  }
  std::cerr << "Unknown tracer type: " << tracer_type << "\n";
  std::terminate();
}

//--------------------------------------------------------------------------------------------------
// BM_SmallSpanReport
//--------------------------------------------------------------------------------------------------
static void BM_SmallSpanReport(benchmark::State& state,
                               const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    for (int i = 0; i < MaxBufferedSpans; ++i) {
      auto span = tracer->StartSpan("abc123");
    }
    tracer->Flush();
  }
}
BENCHMARK_CAPTURE(BM_SmallSpanReport, legacy_manual, "legacy_manual");
BENCHMARK_CAPTURE(BM_SmallSpanReport, manual, "manual");

//--------------------------------------------------------------------------------------------------
// BM_TaggedSpanReport
//--------------------------------------------------------------------------------------------------
static void BM_TaggedSpanReport(benchmark::State& state,
                                const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    for (int i = 0; i < MaxBufferedSpans; ++i) {
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
    tracer->Flush();
  }
}
BENCHMARK_CAPTURE(BM_TaggedSpanReport, legacy_manual, "legacy_manual");
BENCHMARK_CAPTURE(BM_TaggedSpanReport, manual, "manual");

//--------------------------------------------------------------------------------------------------
// BM_LoggedSpanReport
//--------------------------------------------------------------------------------------------------
static void BM_LoggedSpanReport(benchmark::State& state,
                                const char* tracer_type) {
  auto tracer = MakeTracer(tracer_type);
  assert(tracer != nullptr);
  for (auto _ : state) {
    for (int i = 0; i < MaxBufferedSpans; ++i) {
      auto span = tracer->StartSpan("abc123");
      for (int i = 0; i < 10; ++i) {
        span->Log({{"abc", 123}});
      }
    }
    tracer->Flush();
  }
}
BENCHMARK_CAPTURE(BM_LoggedSpanReport, legacy_manual, "legacy_manual");
BENCHMARK_CAPTURE(BM_LoggedSpanReport, manual, "manual");

//--------------------------------------------------------------------------------------------------
// BENCHMARK_MAIN
//--------------------------------------------------------------------------------------------------
BENCHMARK_MAIN();
