#include "test/recorder/in_memory_recorder.h"
#include "test/tracer/propagation/http_headers_carrier.h"
#include "test/tracer/propagation/text_map_carrier.h"
#include "test/tracer/propagation/utility.h"
#include "tracer/immutable_span_context.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("trace-context propagation") {
  LightStepTracerOptions tracer_options;
  tracer_options.propagation_modes = {PropagationMode::trace_context};
  auto recorder = new InMemoryRecorder{};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new TracerImpl{MakePropagationOptions(tracer_options),
                     std::unique_ptr<Recorder>{recorder}}};
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier{text_map};
  HTTPHeadersCarrier http_headers_carrier{text_map};

  SECTION("Inject, extract yields the same span context.") {
    auto test_span_contexts = MakeTestSpanContexts(true);
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

  SECTION("trace_flags is copied over to children") {
    TraceContext trace_context;
    trace_context.trace_id_high = 123;
    trace_context.trace_id_low = 456;
    trace_context.parent_id = 789;
    trace_context.trace_flags = 2;
    ImmutableSpanContext span_context{trace_context, "", BaggageProtobufMap{}};
    auto span = tracer->StartSpan("abc", {opentracing::ChildOf(&span_context)});
    REQUIRE(dynamic_cast<const LightStepSpanContext&>(span->context())
                .trace_flags() == 2);
  }

  SECTION("trace_flags are ORed from multiple parents") {
    TraceContext trace_context;
    trace_context.trace_id_high = 123;
    trace_context.trace_id_low = 456;
    trace_context.parent_id = 789;
    trace_context.trace_flags = 2;
    ImmutableSpanContext span_context1{trace_context, "", BaggageProtobufMap{}};
    trace_context.trace_flags = 1;
    ImmutableSpanContext span_context2{trace_context, "", BaggageProtobufMap{}};
    auto span =
        tracer->StartSpan("abc", {opentracing::ChildOf(&span_context1),
                                  opentracing::ChildOf(&span_context2)});
    REQUIRE(dynamic_cast<const LightStepSpanContext&>(span->context())
                .trace_flags() == 3);
  }

  SECTION("trace_state is copied over to children") {
    TraceContext trace_context;
    trace_context.trace_id_high = 123;
    trace_context.trace_id_low = 456;
    trace_context.parent_id = 789;
    trace_context.trace_flags = 2;
    ImmutableSpanContext span_context{trace_context, "abc=123",
                                      BaggageProtobufMap{}};
    auto span = tracer->StartSpan("abc", {opentracing::ChildOf(&span_context)});
    REQUIRE(dynamic_cast<const LightStepSpanContext&>(span->context())
                .trace_state() == "abc=123");
  }

  SECTION("trace_states are joined from multiple parents") {
    TraceContext trace_context;
    trace_context.trace_id_high = 123;
    trace_context.trace_id_low = 456;
    trace_context.parent_id = 789;
    trace_context.trace_flags = 2;
    ImmutableSpanContext span_context1{trace_context, "abc=123",
                                       BaggageProtobufMap{}};
    trace_context.trace_flags = 1;
    ImmutableSpanContext span_context2{trace_context, "xyz=456",
                                       BaggageProtobufMap{}};
    auto span =
        tracer->StartSpan("abc", {opentracing::ChildOf(&span_context1),
                                  opentracing::ChildOf(&span_context2)});
    REQUIRE(dynamic_cast<const LightStepSpanContext&>(span->context())
                .trace_state() == "abc=123,xyz=456");
  }
}
