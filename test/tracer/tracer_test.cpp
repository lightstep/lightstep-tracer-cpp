#include <opentracing/noop.h>

#include "3rd_party/catch2/catch.hpp"
#include "common/utility.h"
#include "lightstep/tracer.h"
#include "test/recorder/in_memory_recorder.h"
#include "test/utility.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tag.h"
#include "tracer/tracer_impl.h"

using namespace lightstep;
using namespace opentracing;

static std::shared_ptr<opentracing::Tracer> MakeTracer(
    const std::string& name, std::unique_ptr<Recorder>&& recorder) {
  if (name == "legacy") {
    return std::shared_ptr<opentracing::Tracer>{
        new LegacyTracerImpl{PropagationOptions{}, std::move(recorder)}};
  }
  return std::shared_ptr<opentracing::Tracer>{
      new TracerImpl{PropagationOptions{}, std::move(recorder)}};
}

TEST_CASE("tracer") {
  for (std::string tracer_type : {"legacy", "nextgen"}) {
    auto recorder = new InMemoryRecorder{};
    auto tracer = MakeTracer(tracer_type, std::unique_ptr<Recorder>{recorder});

    SECTION(tracer_type + ": StartSpan applies the provided tags.") {
      {
        auto span =
            tracer->StartSpan("a", {SetTag("abc", 123), SetTag("xyz", true)});
        REQUIRE(span);
        span->Finish();
      }
      auto span = recorder->top();
      REQUIRE(span.operation_name() == "a");
      REQUIRE(HasTag(span, "abc", 123));
      REQUIRE(HasTag(span, "xyz", true));
    }

    SECTION(tracer_type +
            ": Sampling of a span can be turned off by setting the "
            "sampling_priority "
            "tag to 0.") {
      {
        auto span = tracer->StartSpan("a", {SetTag(SamplingPriorityKey, 0u)});
        REQUIRE(span);
      }
      {
        auto span = tracer->StartSpan("a");
        span->SetTag(SamplingPriorityKey, 0);
        REQUIRE(span);
      }
      REQUIRE(recorder->size() == 0);
    }

    SECTION(
        tracer_type +
        ": If a span's parent isn't sampled, then it also isn't sampled unless "
        "sampling priority is overwritten.") {
      auto parent_span =
          tracer->StartSpan("a", {SetTag(SamplingPriorityKey, 0u)});
      REQUIRE(parent_span);
      {
        auto child_span = tracer->StartSpan(
            "b", {opentracing::ChildOf(&parent_span->context())});
        REQUIRE(child_span);
      }
      REQUIRE(recorder->size() == 0);
      {
        auto child_span = tracer->StartSpan(
            "b", {opentracing::ChildOf(&parent_span->context())});
        REQUIRE(child_span);
        child_span->SetTag(SamplingPriorityKey, 1);
      }
      REQUIRE(recorder->size() == 1);
    }

    SECTION(tracer_type +
            ": You can set a single child-of reference when starting a span.") {
      auto span_a = tracer->StartSpan("a");
      REQUIRE(span_a);
      span_a->Finish();
      auto span_b = tracer->StartSpan("b", {ChildOf(&span_a->context())});
      REQUIRE(span_b);
      span_b->Finish();
      auto spans = recorder->spans();
      REQUIRE(spans.at(0).span_context().trace_id() ==
              spans.at(1).span_context().trace_id());
      REQUIRE(HasRelationship(SpanReferenceType::ChildOfRef, spans.at(1),
                              spans.at(0)));
    }

    SECTION(
        tracer_type +
        ": You can set a single follows-from reference when starting a span.") {
      auto span_a = tracer->StartSpan("a");
      REQUIRE(span_a);
      span_a->Finish();
      auto span_b = tracer->StartSpan("b", {FollowsFrom(&span_a->context())});
      REQUIRE(span_b);
      span_b->Finish();
      auto spans = recorder->spans();
      REQUIRE(spans.at(0).span_context().trace_id() ==
              spans.at(1).span_context().trace_id());
      REQUIRE(HasRelationship(SpanReferenceType::FollowsFromRef, spans.at(1),
                              spans.at(0)));
    }

    SECTION(tracer_type +
            ": Multiple references are supported when starting a span.") {
      auto span_a = tracer->StartSpan("a");
      REQUIRE(span_a);
      auto span_b = tracer->StartSpan("b");
      REQUIRE(span_b);
      auto span_c = tracer->StartSpan(
          "c", {ChildOf(&span_a->context()), FollowsFrom(&span_b->context())});
      span_a->Finish();
      span_b->Finish();
      span_c->Finish();
      auto spans = recorder->spans();
      REQUIRE(HasRelationship(SpanReferenceType::ChildOfRef, spans.at(2),
                              spans.at(0)));
      REQUIRE(HasRelationship(SpanReferenceType::FollowsFromRef, spans.at(2),
                              spans.at(1)));
    }

    SECTION(tracer_type + ": Baggage is treated case-insensitively") {
      auto span_a = tracer->StartSpan("a");
      REQUIRE(span_a);
      span_a->SetBaggageItem("AbC", "123");
      REQUIRE(span_a->BaggageItem("aBc") == "123");
    }

    SECTION(tracer_type +
            ": Baggage from the span references are copied over to a new span "
            "context") {
      auto span_a = tracer->StartSpan("a");
      REQUIRE(span_a);
      span_a->SetBaggageItem("a", "1");
      auto span_b = tracer->StartSpan("b");
      REQUIRE(span_b);
      span_b->SetBaggageItem("b", "2");
      auto span_c = tracer->StartSpan(
          "c", {ChildOf(&span_a->context()), ChildOf(&span_b->context())});
      REQUIRE(span_c);
      REQUIRE(span_c->BaggageItem("a") == "1");
      REQUIRE(span_c->BaggageItem("b") == "2");
    }

    SECTION(
        tracer_type +
        ": References to non-LightStep spans and null pointers are ignored.") {
      auto noop_tracer = MakeNoopTracer();
      auto noop_span = noop_tracer->StartSpan("noop");
      REQUIRE(noop_span);
      StartSpanOptions options;
      options.references.emplace_back(
          std::make_pair(SpanReferenceType::ChildOfRef, &noop_span->context()));
      options.references.emplace_back(
          std::make_pair(SpanReferenceType::ChildOfRef, nullptr));
      auto span = tracer->StartSpanWithOptions("a", options);
      REQUIRE(span);
      span->Finish();
      REQUIRE(recorder->top().references_size() == 0);
    }

    SECTION(tracer_type + ": Calling Finish a second time does nothing.") {
      auto span = tracer->StartSpan("a");
      REQUIRE(span);
      span->Finish();
      REQUIRE(recorder->size() == 1);
      span->Finish();
      REQUIRE(recorder->size() == 1);
    }

    SECTION(tracer_type +
            ": The operation name can be changed after the span is started.") {
      auto span = tracer->StartSpan("a");
      REQUIRE(span);
      span->SetOperationName("b");
      span->Finish();
      REQUIRE(recorder->top().operation_name() == "b");
    }

    SECTION(tracer_type + ": Tags can be specified after a span is started.") {
      auto span = tracer->StartSpan("a");
      REQUIRE(span);
      span->SetTag("abc", 123);
      span->Finish();
      REQUIRE(HasTag(recorder->top(), "abc", 123));
    }

    SECTION(tracer_type + ": Logs are appended and sent to the collector.") {
      auto span = tracer->StartSpan("a");
      REQUIRE(span);
      span->Log({{"abc", 123}});
      span->Finish();
      REQUIRE(recorder->top().logs().size() == 1);
    }

    SECTION(tracer_type + ": Logs can be added with FinishSpanOptions.") {
      auto span = tracer->StartSpan("a");
      REQUIRE(span);
      opentracing::FinishSpanOptions options;
      options.log_records = {{SystemClock::now(), {{"abc", 123}}}};
      span->FinishWithOptions(options);
      REQUIRE(recorder->top().logs().size() == 1);
    }

    SECTION(tracer_type +
            ": calling operations that modify a span after it's been finished "
            "don't do anything catastrophic") {
      auto span = tracer->StartSpan("a");
      REQUIRE(span);
      span->Finish();
      span->SetTag("abc", 123);
      span->Log({{"abc", 123}});
    }
  }
}

TEST_CASE("Configuration validation") {
  LightStepTracerOptions options;

  // Don't make connections to the outside network; there's nothing
  // listening at these addresses but that's ok since we don't need to send
  // spans with this test.
  options.collector_host = "localhost";
  options.collector_port = 1234;
  options.satellite_endpoints = {{"localhost", 1234}};

  SECTION("We can construct a valid rpc tracer.") {
    REQUIRE(MakeLightStepTracer(std::move(options)) != nullptr);
  }

  SECTION("We can construct a valid streaming tracer.") {
    options.collector_plaintext = true;
    options.use_stream_recorder = true;
    REQUIRE(MakeLightStepTracer(std::move(options)) != nullptr);
  }

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
