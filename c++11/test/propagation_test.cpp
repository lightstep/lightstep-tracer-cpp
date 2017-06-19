#include <lightstep/tracer.h>
#include <opentracing/noop.h>
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
// InvalidCarrier
//------------------------------------------------------------------------------
// A carrier with no implementation for testing errors.
struct InvalidCarrier : CarrierReader, CarrierWriter {};

//------------------------------------------------------------------------------
// tests
//------------------------------------------------------------------------------
TEST_CASE("propagation") {
  auto recorder = new InMemoryRecorder();
  auto tracer = make_lightstep_tracer(std::unique_ptr<Recorder>(recorder));
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier carrier(text_map);
  InvalidCarrier invalid_carrier;

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

  SECTION(
      "Extracing a context from an empty text-map gives a null span context.") {
    auto span_context_maybe = tracer->Extract(CarrierFormat::TextMap, carrier);
    CHECK(span_context_maybe);
    CHECK(span_context_maybe->get() == nullptr);
  }

  SECTION(
      "Injecting a non-LightStep span returns invalid_span_context_error.") {
    auto noop_tracer = make_noop_tracer();
    CHECK(noop_tracer);
    auto span = noop_tracer->StartSpan("a");
    CHECK(span);
    auto was_successful =
        tracer->Inject(span->context(), CarrierFormat::TextMap, carrier);
    CHECK(!was_successful);
    CHECK(was_successful.error() == invalid_span_context_error);
  }

  SECTION(
      "Extracting a span context with missing fields returns "
      "span_context_corrupted_error") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(tracer->Inject(span->context(), CarrierFormat::TextMap, carrier));

    // Remove a field to get an invalid span context.
    text_map.erase(std::begin(text_map));
    auto span_context_maybe = tracer->Extract(CarrierFormat::TextMap, carrier);
    CHECK(!span_context_maybe);
    CHECK(span_context_maybe.error() == span_context_corrupted_error);
  }

  SECTION(
      "Injecting into a non-TextMap carrier returns invalid_carrier_error.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    auto was_successful = tracer->Inject(
        span->context(), CarrierFormat::TextMap, invalid_carrier);
    CHECK(!was_successful);
    CHECK(was_successful.error() == invalid_carrier_error);
  }

  // TODO: Test case sensitivity with HTTPHeader fields.
}
