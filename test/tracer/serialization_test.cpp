#include "tracer/serialization.h"

#include <memory>
#include <sstream>

#include "common/utility.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/message_differencer.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("Serialization") {
  std::ostringstream oss;
  std::unique_ptr<google::protobuf::io::OstreamOutputStream> output_stream{
      new google::protobuf::io::OstreamOutputStream{&oss}};
  google::protobuf::io::CodedOutputStream coded_stream{output_stream.get()};

  collector::Span span;

  auto finalize = [&] {
    coded_stream.Trim();
    output_stream.reset();
    REQUIRE(span.ParseFromString(oss.str()));
  };

  SECTION("We can serialize operation name") {
    WriteOperationName(coded_stream, "abc");
    finalize();
    REQUIRE(span.operation_name() == "abc");
  }

  SECTION("We can reset the operation name") {
    WriteOperationName(coded_stream, "abc");
    WriteOperationName(coded_stream, "abc123");
    finalize();
    REQUIRE(span.operation_name() == "abc123");
  }

  SECTION("We can attach tags to a span") {
    WriteTag(coded_stream, "abc", 123);
    finalize();
    REQUIRE(span.tags().size() == 1);
    auto& tag = span.tags()[0];
    REQUIRE(tag.key() == "abc");
    REQUIRE(tag.int_value() == 123);
  }

  SECTION("We can write the start timestamp") {
    auto now = std::chrono::system_clock::now();
    WriteStartTimestamp(coded_stream, now);
    finalize();
    auto expected_timestamp = ToTimestamp(now);
    REQUIRE(google::protobuf::util::MessageDifferencer::Equals(
        span.start_timestamp(), expected_timestamp));
  }

  SECTION("We can write the span's duration") {
    auto duration = std::chrono::microseconds{153};
    WriteDuration(
        coded_stream,
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            duration));
    finalize();
    REQUIRE(span.duration_micros() == duration.count());
  }

  SECTION("We can write the span's context") {
    uint64_t trace_id = 123;
    uint64_t span_id = 456;
    std::vector<std::pair<std::string, std::string>> baggage = {{"abc", "123"},
                                                                {"xyz", "456"}};
    WriteSpanContext(coded_stream, trace_id, span_id, baggage);
    finalize();
    auto& span_context = span.span_context();
    REQUIRE(span_context.trace_id() == trace_id);
    REQUIRE(span_context.span_id() == span_id);
    REQUIRE(span_context.baggage().size() == 2);
    auto baggage_map = span_context.baggage();
    REQUIRE(baggage_map["abc"] == "123");
    REQUIRE(baggage_map["xyz"] == "456");
  }

  SECTION("We can write span references") {
    WriteSpanReference(coded_stream, opentracing::SpanReferenceType::ChildOfRef,
                       123, 456);
    WriteSpanReference(coded_stream,
                       opentracing::SpanReferenceType::FollowsFromRef, 789,
                       101112);
    finalize();
    auto& references = span.references();
    REQUIRE(references.size() == 2);
    REQUIRE(references[0].relationship() ==
            collector::Reference_Relationship_CHILD_OF);
    REQUIRE(references[0].span_context().trace_id() == 123);
    REQUIRE(references[0].span_context().span_id() == 456);
    REQUIRE(references[1].relationship() ==
            collector::Reference_Relationship_FOLLOWS_FROM);
    REQUIRE(references[1].span_context().trace_id() == 789);
    REQUIRE(references[1].span_context().span_id() == 101112);
  }

  SECTION("We can attach logs to the a span") {
    auto now = std::chrono::system_clock::now();
    std::vector<std::pair<std::string, opentracing::Value>> fields = {
        {"abc", 123},
        {"json1", opentracing::Values{1, 2, 3}},
        {"json2", opentracing::Dictionary{{"abc", 123}}}};
    WriteLog(coded_stream, now, fields.data(), fields.data() + fields.size());
    finalize();
    auto expected_log = ToLog(now, fields.begin(), fields.end());
    REQUIRE(span.logs().size() == 1);
    REQUIRE(google::protobuf::util::MessageDifferencer::Equals(span.logs()[0],
                                                               expected_log));
  }
}
