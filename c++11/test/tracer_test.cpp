#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include "in_memory_tracer.h"

#define CATCH_CONFIG_MAIN
#include <google/protobuf/util/message_differencer.h>
#include <lightstep/catch/catch.hpp>
using namespace lightstep;
using namespace opentracing;

namespace lightstep {
std::shared_ptr<Tracer> make_lightstep_tracer(
    std::unique_ptr<Recorder>&& recorder);

collector::KeyValue to_key_value(StringRef key, const Value& value);
}  // namespace lightstep

//------------------------------------------------------------------------------
// has_tag
//------------------------------------------------------------------------------
static bool has_tag(const collector::Span& span, StringRef key,
                    const Value& value) {
  auto key_value = to_key_value(key, value);
  return std::find_if(
             std::begin(span.tags()), std::end(span.tags()),
             [&](const collector::KeyValue& other) {
               return google::protobuf::util::MessageDifferencer::Equals(
                   key_value, other);
             }) != std::end(span.tags());
}

//------------------------------------------------------------------------------
// has_relationship
//------------------------------------------------------------------------------
static bool has_relationship(SpanReferenceType relationship,
                             const collector::Span& span_a,
                             const collector::Span& span_b) {
  collector::Reference reference;
  switch (relationship) {
    case SpanReferenceType::ChildOfRef:
      reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case SpanReferenceType::FollowsFromRef:
      reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  *reference.mutable_span_context() = span_b.span_context();
  return std::find_if(
             std::begin(span_a.references()), std::end(span_a.references()),
             [&](const collector::Reference& other) {
               return google::protobuf::util::MessageDifferencer::Equals(
                   reference, other);
             }) != std::end(span_a.references());
}

//------------------------------------------------------------------------------
// tests
//------------------------------------------------------------------------------
TEST_CASE("in_memory_tracer") {
  auto recorder = new InMemoryRecorder();
  auto tracer = make_lightstep_tracer(std::unique_ptr<Recorder>(recorder));

  SECTION("StartSpan applies the provided tags.") {
    {
      auto span =
          tracer->StartSpan("a", {SetTag("abc", 123), SetTag("xyz", true)});
      CHECK(span);
      span->Finish();
    }
    auto span = recorder->top();
    CHECK(span.operation_name() == "a");
    CHECK(has_tag(span, "abc", 123));
    CHECK(has_tag(span, "xyz", true));
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
    CHECK(has_relationship(SpanReferenceType::ChildOfRef, spans.at(1),
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
    CHECK(has_relationship(SpanReferenceType::FollowsFromRef, spans.at(1),
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
    CHECK(has_relationship(SpanReferenceType::ChildOfRef, spans.at(2),
                           spans.at(0)));
    CHECK(has_relationship(SpanReferenceType::FollowsFromRef, spans.at(2),
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
    auto noop_tracer = make_noop_tracer();
    auto noop_span = noop_tracer->StartSpan("noop");
    CHECK(noop_span);
    StartSpanOptions options;
    options.references.push_back(
        std::make_pair(SpanReferenceType::ChildOfRef, &noop_span->context()));
    options.references.push_back(
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

  SECTION("Tags can be specified.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetTag("abc", 123);
    span->Finish();
    CHECK(has_tag(recorder->top(), "abc", 123));
  }
}
