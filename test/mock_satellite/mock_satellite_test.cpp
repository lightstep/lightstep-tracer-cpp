#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/http_connection.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("MockSatellite") {
  MockSatelliteHandle mock_satellite{
      static_cast<uint16_t>(PortAssignments::MockSatelliteTest)};
  HttpConnection connection{
      "127.0.0.1", static_cast<uint16_t>(PortAssignments::MockSatelliteTest)};

  SECTION("ReportRequests can be posted to MockSatellite") {
    collector::ReportRequest report;
    collector::Span span1;
    span1.mutable_span_context()->set_span_id(1);
    *report.mutable_spans()->Add() = span1;
    collector::ReportResponse response;
    connection.Post("/report", report, response);
    REQUIRE(mock_satellite.spans().size() == 1);
  }
}
