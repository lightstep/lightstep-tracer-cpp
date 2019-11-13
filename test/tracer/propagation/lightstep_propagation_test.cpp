#include <algorithm>

#include "test/recorder/in_memory_recorder.h"
#include "test/tracer/propagation/http_headers_carrier.h"
#include "test/tracer/propagation/text_map_carrier.h"
#include "test/tracer/propagation/utility.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("lightstep propagation") {
  LightStepTracerOptions tracer_options;
  tracer_options.propagation_modes = {PropagationMode::b3,
                                      PropagationMode::lightstep};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new TracerImpl{MakePropagationOptions(tracer_options),
                     std::unique_ptr<Recorder>{new InMemoryRecorder{}}}};
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier{text_map};
  HTTPHeadersCarrier http_headers_carrier{text_map};

  SECTION(
      "If a 128-bit trace id is injected via lightstep, it truncates to use "
      "the lower part of the id") {
    text_map = {{"x-b3-traceid", "aef5705a090040838f1359ebafa5c0c6"},
                {"x-b3-spanid", "aef5705a09004083"}};
    auto span_context_maybe = tracer->Extract(http_headers_carrier);
    REQUIRE(span_context_maybe);
    text_map.clear();
    REQUIRE(tracer->Inject(**span_context_maybe, text_map_carrier));
    REQUIRE(text_map["ot-tracer-traceid"] == "8f1359ebafa5c0c6");
  }
}
