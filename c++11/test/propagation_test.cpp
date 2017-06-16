#include <lightstep/tracer.h>
#include <string>
#include <unordered_map>
#include "in_memory_tracer.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

namespace lightstep {
std::shared_ptr<Tracer> make_lightstep_tracer(
    std::unique_ptr<Recorder>&& recorder);

collector::KeyValue to_key_value(StringRef key, const Value& value);
}  // namespace lightstep

//------------------------------------------------------------------------------
// TextMapCarrier
//------------------------------------------------------------------------------
struct TextMapCarrier : TextMapReader, TextMapWriter {
  TextMapCarrier(std::unordered_map<std::string, std::string>& text_map)
      : text_map(text_map) {}

  Expected<void> Set(const std::string& key,
                     const std::string& value) const override {
    text_map[key] = value;
    return {};
  }

  Expected<void> ForeachKey(
      std::function<Expected<void>(StringRef key, StringRef value)> f)
      const override {
    for (const auto& key_value : text_map) {
      auto result = f(key_value.first, key_value.second);
      if (!result) return result;
    }
    return {};
  }

  std::unordered_map<std::string, std::string>& text_map;
};

//------------------------------------------------------------------------------
// tests
//------------------------------------------------------------------------------
TEST_CASE("propagation") {
  auto recorder = new InMemoryRecorder();
  auto tracer = make_lightstep_tracer(std::unique_ptr<Recorder>(recorder));
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier carrier(text_map);

  SECTION("Inject, extract, inject yields the same text_map.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(tracer->Inject(span->context(), CarrierFormat::TextMap, carrier));
    auto injection_map1 = text_map;
    auto span_context_maybe = tracer->Extract(CarrierFormat::TextMap, carrier);
    CHECK(span_context_maybe);
    auto span_context = std::move(*span_context_maybe);
    CHECK(span_context);
    text_map.clear();
    CHECK(tracer->Inject(*span_context, CarrierFormat::TextMap, carrier));
    CHECK(injection_map1 == text_map);
  }
}
