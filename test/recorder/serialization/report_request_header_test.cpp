#include "recorder/serialization/report_request_header.h"

#include "3rd_party/catch2/catch.hpp"
#include "lightstep-tracer-common/collector.pb.h"
using namespace lightstep;

TEST_CASE("WriteReportRequestHeader") {
  LightStepTracerOptions tracer_options;
  tracer_options.access_token = "abc123";
  tracer_options.tags = {{"xyz", 456}};
  auto serialization = WriteReportRequestHeader(tracer_options, 789);
  collector::ReportRequest report;
  REQUIRE(report.ParseFromString(serialization));
  REQUIRE(report.auth().access_token() == "abc123");
  REQUIRE(report.reporter().reporter_id() == 789);
  auto& tags = report.reporter().tags();
  REQUIRE(tags.size() == 1);
  REQUIRE(tags[0].key() == "xyz");
  REQUIRE(tags[0].int_value() == 456);
}
