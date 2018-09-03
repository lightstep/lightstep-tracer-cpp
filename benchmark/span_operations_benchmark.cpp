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

BENCHMARK_MAIN();
