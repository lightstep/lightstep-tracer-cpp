#include <google/protobuf/util/message_differencer.h>
#include <opentracing/noop.h>
#include <algorithm>
#include <cctype>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include "common/utility.h"
#include "lightstep/binary_carrier.h"
#include "lightstep/tracer.h"
#include "test/recorder/in_memory_recorder.h"
#include "test/tracer/propagation/http_headers_carrier.h"
#include "test/tracer/propagation/text_map_carrier.h"
#include "test/tracer/propagation/utility.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

//------------------------------------------------------------------------------
// LightStepBinaryReaderWriter
//------------------------------------------------------------------------------
class LightStepBinaryReaderWriter : public LightStepBinaryReader,
                                    public LightStepBinaryWriter {
 public:
  explicit LightStepBinaryReaderWriter(lightstep::BinaryCarrier& carrier)
      : LightStepBinaryReader{&carrier}, LightStepBinaryWriter{carrier} {}
};

//--------------------------------------------------------------------------------------------------
// MakeTracers
//--------------------------------------------------------------------------------------------------
static std::vector<std::pair<std::string, std::shared_ptr<opentracing::Tracer>>>
MakeTracers(const LightStepTracerOptions& tracer_options = {}) {
  std::vector<std::pair<std::string, std::shared_ptr<opentracing::Tracer>>>
      result;
  auto legacy_tracer = std::shared_ptr<opentracing::Tracer>{
      new LegacyTracerImpl{MakePropagationOptions(tracer_options),
                           std::unique_ptr<Recorder>{new InMemoryRecorder{}}}};
  result.emplace_back("LegacyTracer", legacy_tracer);

  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new TracerImpl{MakePropagationOptions(tracer_options),
                     std::unique_ptr<Recorder>{new InMemoryRecorder{}}}};
  result.emplace_back("Tracer", tracer);
  return result;
}

//------------------------------------------------------------------------------
// tests
//------------------------------------------------------------------------------
TEST_CASE("propagation") {
  for (auto named_tracer : MakeTracers()) {
    SECTION(named_tracer.first) {
      auto tracer = named_tracer.second;
      std::unordered_map<std::string, std::string> text_map;
      TextMapCarrier text_map_carrier{text_map};
      HTTPHeadersCarrier http_headers_carrier{text_map};
      BinaryCarrier binary_carrier;
      LightStepBinaryReaderWriter binary_reader_writer{binary_carrier};

      SECTION("Inject, extract yields the same span context.") {
        auto test_span_contexts = MakeTestSpanContexts();
        for (auto& span_context : test_span_contexts) {
          // text map carrier
          CHECK_NOTHROW(
              VerifyInjectExtract(*tracer, *span_context, text_map_carrier));
          text_map.clear();

          // http headers carrier
          CHECK_NOTHROW(VerifyInjectExtract(*tracer, *span_context,
                                            http_headers_carrier));
          text_map.clear();

          // iostream carrier
          std::stringstream ss;
          CHECK_NOTHROW(VerifyInjectExtract(*tracer, *span_context, ss));

          // binary carrier
          CHECK_NOTHROW(VerifyInjectExtract(*tracer, *span_context,
                                            binary_reader_writer));
          // Note: binary injection clears the existing values so no need to
          // reset.
        }
      }

      SECTION(
          "Extracing a context from an empty text-map gives a null span "
          "context.") {
        auto span_context_maybe = tracer->Extract(text_map_carrier);
        CHECK(span_context_maybe);
        CHECK(span_context_maybe->get() == nullptr);
      }

      SECTION(
          "Injecting a non-LightStep span returns "
          "invalid_span_context_error.") {
        auto noop_tracer = opentracing::MakeNoopTracer();
        CHECK(noop_tracer);
        auto span = noop_tracer->StartSpan("a");
        CHECK(span);
        auto was_successful = tracer->Inject(span->context(), text_map_carrier);
        CHECK(!was_successful);
        CHECK(was_successful.error() ==
              opentracing::invalid_span_context_error);
      }

      SECTION(
          "Extracting a span context with missing fields returns "
          "span_context_corrupted_error") {
        auto span = tracer->StartSpan("a");
        CHECK(span);
        CHECK(tracer->Inject(span->context(), text_map_carrier));

        // Remove a field to get an invalid span context.
        auto iter = text_map.find("ot-tracer-traceid");
        REQUIRE(iter != text_map.end());
        text_map.erase(iter);

        for (auto& kv : text_map) {
          std::cout << kv.first << ": " << kv.second << std::endl;
        }
        auto span_context_maybe = tracer->Extract(text_map_carrier);
        CHECK(!span_context_maybe);
        CHECK(span_context_maybe.error() ==
              opentracing::span_context_corrupted_error);
      }

      SECTION(
          "Extract is insensitive to changes in case for http header fields") {
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

      SECTION(
          "Extracting a span from an invalid binary blob returns "
          "span_context_corrupted_error.") {
        std::string invalid_context = "abc123";
        std::istringstream iss{invalid_context, std::ios::binary};
        auto span_context_maybe = tracer->Extract(iss);
        CHECK(!span_context_maybe);
        CHECK(span_context_maybe.error() ==
              opentracing::span_context_corrupted_error);
      }

      SECTION("Calling Extract on an empty stream yields a nullptr.") {
        std::string blob;
        std::istringstream iss(blob, std::ios::binary);
        auto span_context_maybe = tracer->Extract(iss);
        CHECK(span_context_maybe);
        CHECK(span_context_maybe->get() == nullptr);
      }
    }
  }
}
