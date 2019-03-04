#include <sstream>
#include <cstdint>

#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/http_connection.h"
#include "3rd_party/catch2/catch.hpp"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
using namespace lightstep;

static void WriteEmbeddedMessage(
    google::protobuf::io::CodedOutputStream& stream, uint32_t field,
    const google::protobuf::Message& message) {
  stream.WriteVarint32((field << 3) | 2);
  stream.WriteVarint64(message.ByteSizeLong());
  message.SerializeToCodedStream(&stream);
}

static std::string WriteStreamingReport(const collector::ReportRequest& report) {
  std::ostringstream oss;
  {
    google::protobuf::io::OstreamOutputStream zero_copy_stream{&oss};
    google::protobuf::io::CodedOutputStream coded_stream{&zero_copy_stream};
    WriteEmbeddedMessage(coded_stream, 1, report.reporter());
    WriteEmbeddedMessage(coded_stream, 2, report.auth());
    WriteEmbeddedMessage(coded_stream, 6, report.internal_metrics());
    for (auto& span : report.spans()) {
      WriteEmbeddedMessage(coded_stream, 3, span);
    }
  }
  return oss.str();
}

TEST_CASE("MockSatellite") {
  MockSatelliteHandle mock_satellite{
      static_cast<uint16_t>(PortAssignments::MockSatelliteTest)};
  HttpConnection connection{
      "127.0.0.1", static_cast<uint16_t>(PortAssignments::MockSatelliteTest)};

  SECTION("ReportRequests can be posted to a MockSatellite") {
    collector::ReportRequest report;
    collector::Span span1;
    span1.mutable_span_context()->set_span_id(1);
    *report.mutable_spans()->Add() = span1;
    collector::ReportResponse response;
    connection.Post("/report", report, response);
    REQUIRE(mock_satellite.spans().size() == 1);
  }

  SECTION("ReportRequests can be streamed to a MockSatellite") {
    collector::ReportRequest report;
    report.mutable_reporter()->set_reporter_id(123);
    report.mutable_auth()->set_access_token("abc");
    report.mutable_internal_metrics()->set_duration_micros(321);

    collector::Span span1;
    span1.mutable_span_context()->set_span_id(1);
    *report.mutable_spans()->Add() = span1;

    collector::Span span2;
    span1.mutable_span_context()->set_span_id(2);
    *report.mutable_spans()->Add() = span2;

    auto s = WriteStreamingReport(report);

    collector::ReportResponse response;
    connection.Post("/report-mock-streaming", s, response);
    REQUIRE(mock_satellite.spans().size() == 2);
    REQUIRE(mock_satellite.reports().size() == 3);
  }
}
