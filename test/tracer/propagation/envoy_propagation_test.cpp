#include <algorithm>

#include "test/recorder/in_memory_recorder.h"
#include "test/tracer/propagation/http_headers_carrier.h"
#include "test/tracer/propagation/text_map_carrier.h"
#include "test/tracer/propagation/utility.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("envoy propagation") {
  LightStepTracerOptions tracer_options;
  tracer_options.use_single_key_propagation = true;
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new TracerImpl{MakePropagationOptions(tracer_options),
                     std::unique_ptr<Recorder>{new InMemoryRecorder{}}}};
  auto multikey_tracer = std::shared_ptr<opentracing::Tracer>{
      new LegacyTracerImpl{MakePropagationOptions(LightStepTracerOptions{}),
                           std::unique_ptr<Recorder>{new InMemoryRecorder{}}}};
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier{text_map};
  HTTPHeadersCarrier http_headers_carrier{text_map};

  SECTION("Inject, extract yields the same span context.") {
    auto test_span_contexts = MakeTestSpanContexts();
    for (auto& span_context : test_span_contexts) {
      // text map carrier
      CHECK_NOTHROW(
          VerifyInjectExtract(*tracer, *span_context, text_map_carrier));
      text_map.clear();

      // http headers carrier
      CHECK_NOTHROW(
          VerifyInjectExtract(*tracer, *span_context, http_headers_carrier));
      text_map.clear();
    }
  }

  auto span = tracer->StartSpan("a");
  CHECK(span);
  span->SetBaggageItem("abc", "123");
  CHECK(tracer->Inject(span->context(), text_map_carrier));
  CHECK(text_map.size() == 1);

  SECTION("If a carrier supports LookupKey, then ForeachKey won't be called") {
    text_map_carrier.supports_lookup = true;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get()));
    CHECK(text_map_carrier.foreach_key_call_count == 0);
  }

  SECTION(
      "Extract falls back to the multikey format if it doesn't find a "
      "single-key header") {
    text_map.clear();
    CHECK(multikey_tracer->Inject(span->context(), text_map_carrier));
    CHECK(text_map.size() > 1);
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get()));
  }

  SECTION(
      "When LookupKey is used, a nullptr is returned if there is no "
      "span_context") {
    text_map.clear();
    text_map_carrier.supports_lookup = true;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get() == nullptr));

    // We have to iterate through all of the keys once when extraction falls
    // back to the multikey format.
    CHECK(text_map_carrier.foreach_key_call_count == 1);
  }

  SECTION("Verify only valid base64 characters are used.") {
    // Follows the guidelines given in RFC-4648 on what characters are
    // permissible. See
    //    http://www.rfc-editor.org/rfc/rfc4648.txt
    auto iter = text_map.begin();
    CHECK(iter != text_map.end());
    auto value = iter->second;
    auto is_base64_char = [](char c) {
      return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
             ('0' <= c && c <= '9') || c == '+' || c == '/' || c == '=';
    };
    CHECK(std::all_of(value.begin(), value.end(), is_base64_char));
    CHECK(value.size() % 4 == 0);
  }
}
