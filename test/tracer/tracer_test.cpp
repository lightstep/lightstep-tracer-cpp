#include <opentracing/ext/tags.h>
#include <opentracing/noop.h>

#include "3rd_party/catch2/catch.hpp"
#include "common/utility.h"
#include "lightstep/tracer.h"
#include "test/recorder/in_memory_recorder.h"
#include "test/utility.h"
#include "tracer/lightstep_tracer_impl.h"

using namespace lightstep;
using namespace opentracing;

TEST_CASE("tracer") {
  auto recorder = new InMemoryRecorder{};
  auto tracer = std::shared_ptr<opentracing::Tracer>{new LightStepTracerImpl{
      PropagationOptions{}, std::unique_ptr<Recorder>{recorder}}};

  SECTION("StartSpan applies the provided tags.") {
    {
      auto span =
          tracer->StartSpan("a", {SetTag("abc", 123), SetTag("xyz", true)});
      CHECK(span);
      span->Finish();
    }
    auto span = recorder->top();
    CHECK(span.operation_name() == "a");
    CHECK(HasTag(span, "abc", 123));
    CHECK(HasTag(span, "xyz", true));
  }

  SECTION(
      "Sampling of a span can be turned off by setting the sampling_priority "
      "tag to 0.") {
    {
      auto span = tracer->StartSpan(
          "a", {SetTag(opentracing::ext::sampling_priority, 0u)});
      CHECK(span);
    }
    {
      auto span = tracer->StartSpan("a");
      span->SetTag(opentracing::ext::sampling_priority, 0);
      CHECK(span);
    }
    CHECK(recorder->size() == 0);
  }

  SECTION(
      "If a span's parent isn't sampled, then it also isn't sampled unless "
      "sampling priority is overwritten.") {
    auto parent_span = tracer->StartSpan(
        "a", {SetTag(opentracing::ext::sampling_priority, 0u)});
    CHECK(parent_span);
    {
      auto child_span = tracer->StartSpan(
          "b", {opentracing::ChildOf(&parent_span->context())});
      CHECK(child_span);
    }
    CHECK(recorder->size() == 0);
    {
      auto child_span = tracer->StartSpan(
          "b", {opentracing::ChildOf(&parent_span->context())});
      CHECK(child_span);
      child_span->SetTag(opentracing::ext::sampling_priority, 1);
    }
    CHECK(recorder->size() == 1);
  }

  SECTION("You can set a single child-of reference when starting a span.") {
    auto span_a = tracer->StartSpan("a");
    CHECK(span_a);
    span_a->Finish();
    auto span_b = tracer->StartSpan("b", {ChildOf(&span_a->context())});
    CHECK(span_b);
    span_b->Finish();
    auto spans = recorder->spans();
    CHECK(spans.at(0).span_context().trace_id() ==
          spans.at(1).span_context().trace_id());
    CHECK(HasRelationship(SpanReferenceType::ChildOfRef, spans.at(1),
                          spans.at(0)));
  }

  SECTION("You can set a single follows-from reference when starting a span.") {
    auto span_a = tracer->StartSpan("a");
    CHECK(span_a);
    span_a->Finish();
    auto span_b = tracer->StartSpan("b", {FollowsFrom(&span_a->context())});
    CHECK(span_b);
    span_b->Finish();
    auto spans = recorder->spans();
    CHECK(spans.at(0).span_context().trace_id() ==
          spans.at(1).span_context().trace_id());
    CHECK(HasRelationship(SpanReferenceType::FollowsFromRef, spans.at(1),
                          spans.at(0)));
  }

  SECTION("Multiple references are supported when starting a span.") {
    auto span_a = tracer->StartSpan("a");
    CHECK(span_a);
    auto span_b = tracer->StartSpan("b");
    CHECK(span_b);
    auto span_c = tracer->StartSpan(
        "c", {ChildOf(&span_a->context()), FollowsFrom(&span_b->context())});
    span_a->Finish();
    span_b->Finish();
    span_c->Finish();
    auto spans = recorder->spans();
    CHECK(HasRelationship(SpanReferenceType::ChildOfRef, spans.at(2),
                          spans.at(0)));
    CHECK(HasRelationship(SpanReferenceType::FollowsFromRef, spans.at(2),
                          spans.at(1)));
  }

  SECTION(
      "Baggage from the span references are copied over to a new span "
      "context") {
    auto span_a = tracer->StartSpan("a");
    CHECK(span_a);
    span_a->SetBaggageItem("a", "1");
    auto span_b = tracer->StartSpan("b");
    CHECK(span_b);
    span_b->SetBaggageItem("b", "2");
    auto span_c = tracer->StartSpan(
        "c", {ChildOf(&span_a->context()), ChildOf(&span_b->context())});
    CHECK(span_c);
    CHECK(span_c->BaggageItem("a") == "1");
    CHECK(span_c->BaggageItem("b") == "2");
  }

  SECTION("References to non-LightStep spans and null pointers are ignored.") {
    auto noop_tracer = MakeNoopTracer();
    auto noop_span = noop_tracer->StartSpan("noop");
    CHECK(noop_span);
    StartSpanOptions options;
    options.references.emplace_back(
        std::make_pair(SpanReferenceType::ChildOfRef, &noop_span->context()));
    options.references.emplace_back(
        std::make_pair(SpanReferenceType::ChildOfRef, nullptr));
    auto span = tracer->StartSpanWithOptions("a", options);
    CHECK(span);
    span->Finish();
    CHECK(recorder->top().references_size() == 0);
  }

  SECTION("Calling Finish a second time does nothing.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->Finish();
    CHECK(recorder->size() == 1);
    span->Finish();
    CHECK(recorder->size() == 1);
  }

  SECTION("The operation name can be changed after the span is started.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetOperationName("b");
    span->Finish();
    CHECK(recorder->top().operation_name() == "b");
  }

  SECTION("Tags can be specified after a span is started.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetTag("abc", 123);
    span->Finish();
    CHECK(HasTag(recorder->top(), "abc", 123));
  }

  SECTION("Logs are appended and sent to the collector.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->Log({{"abc", 123}});
    span->Finish();
    CHECK(recorder->top().logs().size() == 1);
  }

  SECTION("Logs can be added with FinishSpanOptions.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    opentracing::FinishSpanOptions options;
    options.log_records = {{SystemClock::now(), {{"abc", 123}}}};
    span->FinishWithOptions(options);
    CHECK(recorder->top().logs().size() == 1);
  }
}

TEST_CASE("Configuration validation") {
  LightStepTracerOptions options;

  // Don't make connections to the outside network; there's nothing listening at
  // these addresses but that's ok since we don't need to send spans with this
  // test.
  options.collector_host = "localhost";
  options.collector_port = 1234;
  options.satellite_endpoints = {{"localhost", 1234}};

  SECTION(
      "Constructing the streaming tracer errors if plaintext isn't "
      "specified.") {
    options.collector_plaintext = false;
    options.use_stream_recorder = true;
    REQUIRE(MakeLightStepTracer(std::move(options)) == nullptr);
  }

  SECTION("The streaming recorder only supports a threaded mode.") {
    options.use_stream_recorder = true;
    options.use_thread = false;
    REQUIRE(MakeLightStepTracer(std::move(options)) == nullptr);
  }

  SECTION("The streaming recorder doesn't support custom transporters.") {
    options.use_stream_recorder = true;
    options.transporter.reset(new Transporter{});
    REQUIRE(MakeLightStepTracer(std::move(options)) == nullptr);
  }
}
