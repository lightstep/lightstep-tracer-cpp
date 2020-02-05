#include "recorder/serialization/report_request.h"

#include <array>

#include "3rd_party/catch2/catch.hpp"
#include "common/report_request_framing.h"
#include "lightstep-tracer-common/collector.pb.h"
#include "recorder/serialization/report_request_header.h"

#include <google/protobuf/io/coded_stream.h>
using namespace lightstep;

static collector::ReportRequest ToProtobufReportRequest(
    const ReportRequest& report_request) {
  std::string s(report_request.num_bytes(), ' ');
  report_request.CopyOut(&s[0], s.size());
  collector::ReportRequest result;
  if (!result.ParseFromString(s)) {
    std::cerr << "Failed to parse report request\n";
    std::terminate();
  }
  return result;
}

static std::unique_ptr<ChainedStream> ToSerialization(
    const collector::Span& span) {
  auto s = span.SerializeAsString();
  std::array<char, ReportRequestSpansMaxHeaderSize> buffer;
  auto header_size = WriteReportRequestSpansHeader(
      buffer.data(), buffer.size(), static_cast<uint32_t>(s.size()));
  s.insert(0, buffer.data() + (buffer.size() - header_size), header_size);

  std::unique_ptr<ChainedStream> result{new ChainedStream{}};
  std::unique_ptr<google::protobuf::io::CodedOutputStream> stream{
      new google::protobuf::io::CodedOutputStream{result.get()}};
  stream->WriteRaw(static_cast<const void*>(s.data()),
                   static_cast<int>(s.size()));
  stream.reset();
  result->CloseOutput();

  return result;
}

TEST_CASE("ReportRequest") {
  LightStepTracerOptions options;
  uint64_t reporter_id = 123;
  auto header = std::make_shared<std::string>(
      WriteReportRequestHeader(options, reporter_id));
  ReportRequest report_request{header, 0};
  REQUIRE(report_request.num_dropped_spans() == 0);
  REQUIRE(report_request.num_spans() == 0);

  SECTION("We can serialize a ReportRequest with no spans") {
    auto protobuf_report_request = ToProtobufReportRequest(report_request);
    REQUIRE(protobuf_report_request.reporter().reporter_id() == 123);
  }

  SECTION("We can add metrics to a ReportRequest") {
    report_request = ReportRequest{header, 42};
    REQUIRE(report_request.num_dropped_spans() == 42);
    auto protobuf_report_request = ToProtobufReportRequest(report_request);
    auto& dropped_span_count =
        protobuf_report_request.internal_metrics().counts()[0];
    REQUIRE(dropped_span_count.name() == "spans.dropped");
    REQUIRE(dropped_span_count.int_value() == 42);
  }

  SECTION("We can add a span to the ReportRequest") {
    collector::Span span;
    span.mutable_span_context()->set_trace_id(1);
    report_request.AddSpan(ToSerialization(span));

    REQUIRE(report_request.num_spans() == 1);

    auto protobuf_report_request = ToProtobufReportRequest(report_request);
    REQUIRE(protobuf_report_request.spans().size() == 1);
    REQUIRE(protobuf_report_request.spans()[0].span_context().trace_id() == 1);
  }

  SECTION("We can add mulltiple spans to the ReportRequest") {
    collector::Span span1;
    span1.mutable_span_context()->set_trace_id(1);
    report_request.AddSpan(ToSerialization(span1));

    collector::Span span2;
    span2.mutable_span_context()->set_trace_id(2);
    report_request.AddSpan(ToSerialization(span2));

    REQUIRE(report_request.num_spans() == 2);

    auto protobuf_report_request = ToProtobufReportRequest(report_request);
    REQUIRE(protobuf_report_request.spans().size() == 2);
    REQUIRE(protobuf_report_request.spans()[0].span_context().trace_id() == 1);
    REQUIRE(protobuf_report_request.spans()[1].span_context().trace_id() == 2);
  }
}
