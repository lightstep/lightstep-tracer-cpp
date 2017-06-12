#include "in_memory_tracer.h"
#include <lightstep/tracer.h>

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>
#include <google/protobuf/util/message_differencer.h>
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
static bool has_tag(const collector::Span& span, StringRef key, const Value& value) {
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
    const collector::Span& span_a, const collector::Span& span_b) {
  return false;
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

  SECTION("calling Finish a second time does nothing.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->Finish();
    CHECK(recorder->size() == 1);
    span->Finish();
    CHECK(recorder->size() == 1);
  }

  SECTION("the operation name can be set after the span is started.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetOperationName("b");
    span->Finish();
    CHECK(recorder->top().operation_name() == "b");
  }
}
