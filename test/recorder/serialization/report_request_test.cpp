#include "recorder/serialization/report_request.h"

#include "recorder/serialization/report_request_header.h"
#include "3rd_party/catch2/catch.hpp"
#include "lightstep-tracer-common/collector.pb.h"
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

TEST_CASE("ReportRequest") {
  LightStepTracerOptions options;
  uint64_t reporter_id = 123;
  auto header = std::make_shared<std::string>(
      WriteReportRequestHeader(options, reporter_id));
  ReportRequest report_request{header, 0};
  REQUIRE(report_request.num_dropped_spans() == 0);

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
}
