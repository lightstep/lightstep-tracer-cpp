#include <google/protobuf/util/message_differencer.h>
#include <lightstep/binary_carrier.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>
#include "in_memory_tracer.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

namespace lightstep {
std::shared_ptr<Tracer> MakeLightStepTracer(
    std::unique_ptr<Recorder>&& recorder);

collector::KeyValue to_key_value(StringRef key, const Value& value);
}  // namespace lightstep

//------------------------------------------------------------------------------
// TextMapCarrier
//------------------------------------------------------------------------------
struct TextMapCarrier : TextMapReader, TextMapWriter {
  TextMapCarrier(std::unordered_map<std::string, std::string>& text_map_)
      : text_map(text_map_) {}

  Expected<void> Set(StringRef key, StringRef value) const override {
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
// HTTPHeadersCarrier
//------------------------------------------------------------------------------
struct HTTPHeadersCarrier : HTTPHeadersReader, HTTPHeadersWriter {
  HTTPHeadersCarrier(std::unordered_map<std::string, std::string>& text_map_)
      : text_map(text_map_) {}

  Expected<void> Set(StringRef key, StringRef value) const override {
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
  auto tracer = MakeLightStepTracer(std::unique_ptr<Recorder>(recorder));
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier(text_map);
  HTTPHeadersCarrier http_headers_carrier(text_map);
  BinaryCarrier binary_carrier;

  SECTION("Inject, extract, inject yields the same text_map.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(tracer->Inject(span->context(), text_map_carrier));
    auto injection_map1 = text_map;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get()));
    text_map.clear();
    CHECK(tracer->Inject(*span_context_maybe->get(), text_map_carrier));
    CHECK(injection_map1 == text_map);
  }

  SECTION("Inject, extract, inject yields the same BinaryCarrier.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(
        tracer->Inject(span->context(), LightStepBinaryWriter(binary_carrier)));
    auto binary_carrier1 = binary_carrier;
    auto span_context_maybe =
        tracer->Extract(LightStepBinaryReader(&binary_carrier));
    CHECK((span_context_maybe && span_context_maybe->get()));
    CHECK(tracer->Inject(*span_context_maybe->get(),
                         LightStepBinaryWriter(binary_carrier)));
    CHECK(google::protobuf::util::MessageDifferencer::Equals(binary_carrier1,
                                                             binary_carrier));
  }

  SECTION("Inject, extract, inject yields the same binary blob.") {
    std::ostringstream oss(std::ios::binary);
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(tracer->Inject(span->context(), oss));
    auto blob = oss.str();
    std::istringstream iss(blob, std::ios::binary);
    auto span_context_maybe = tracer->Extract(iss);
    CHECK((span_context_maybe && span_context_maybe->get()));
    std::ostringstream oss2(std::ios::binary);
    CHECK(tracer->Inject(*span_context_maybe->get(), oss2));
    CHECK(blob == oss2.str());
  }

  SECTION(
      "Extracing a context from an empty text-map gives a null span context.") {
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK(span_context_maybe);
    CHECK(span_context_maybe->get() == nullptr);
  }

  SECTION(
      "Injecting a non-LightStep span returns invalid_span_context_error.") {
    auto noop_tracer = MakeNoopTracer();
    CHECK(noop_tracer);
    auto span = noop_tracer->StartSpan("a");
    CHECK(span);
    auto was_successful = tracer->Inject(span->context(), text_map_carrier);
    CHECK(!was_successful);
    CHECK(was_successful.error() == invalid_span_context_error);
  }

  SECTION(
      "Extracting a span context with missing fields returns "
      "span_context_corrupted_error") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(tracer->Inject(span->context(), text_map_carrier));

    // Remove a field to get an invalid span context.
    text_map.erase(std::begin(text_map));
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK(!span_context_maybe);
    CHECK(span_context_maybe.error() == span_context_corrupted_error);
  }

  SECTION("Extract is insensitive to changes in case for http header fields") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(tracer->Inject(span->context(), http_headers_carrier));

    // Change the case of one of the fields.
    auto key_value = *std::begin(text_map);
    text_map.erase(std::begin(text_map));
    auto key = key_value.first;
    key[0] = key[0] == std::toupper(key[0])
                 ? static_cast<char>(std::tolower(key[0]))
                 : static_cast<char>(std::toupper(key[0]));
    text_map[key] = key_value.second;
    CHECK(tracer->Extract(http_headers_carrier));
  }

  SECTION("Extract/Inject fail if a stream has failure bits set.") {
    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios_base::failbit);
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(!tracer->Inject(span->context(), oss));
    oss.clear();
    CHECK(tracer->Inject(span->context(), oss));
    auto blob = oss.str();
    std::istringstream iss(blob, std::ios::binary);
    iss.setstate(std::ios_base::failbit);
    CHECK(!tracer->Extract(iss));
  }

  SECTION("Calling Extract on an empty stream yields a nullptr.") {
    std::string blob;
    std::istringstream iss(blob, std::ios::binary);
    auto span_context_maybe = tracer->Extract(iss);
    CHECK(span_context_maybe);
    CHECK(span_context_maybe->get() == nullptr);
  }
}
