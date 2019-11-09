#include <algorithm>

#include "test/recorder/in_memory_recorder.h"
#include "test/tracer/propagation/http_headers_carrier.h"
#include "test/tracer/propagation/text_map_carrier.h"
#include "test/tracer/propagation/utility.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("b3 propagation") {
  LightStepTracerOptions tracer_options;
  tracer_options.propagation_modes = {PropagationMode::b3};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new TracerImpl{MakePropagationOptions(tracer_options),
                     std::unique_ptr<Recorder>{new InMemoryRecorder{}}}};
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier{text_map};
  HTTPHeadersCarrier http_headers_carrier{text_map};

  SECTION("Inject, extract yields the same span context.") {
    for (auto use_128bit_trace_ids : {true, false}) {
      auto test_span_contexts = MakeTestSpanContexts(use_128bit_trace_ids);
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
  }

  SECTION("Verify extraction against specific values") {
  }

  SECTION("A child keeps the same trace id as its parent") {
  }
}
