#include "common/report_request_framing.h"

#include "lightstep-tracer-common/collector.pb.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("ReportRequestFraming") {
  std::string header_serialization(ReportRequestSpansMaxHeaderSize, ' ');

  SECTION("We can successfully parse out serialized spans") {
    collector::ReportRequest report;
    collector::Span span;
    span.mutable_span_context()->set_trace_id(123);
    auto s = span.SerializeAsString();
    auto header_size = WriteReportRequestSpansHeader(
        &header_serialization[0], header_serialization.size(), s.size());
    s = header_serialization.substr(header_serialization.size() - header_size) +
        s;
    REQUIRE(report.ParseFromString(s));
    REQUIRE(report.spans().size() == 1);
    REQUIRE(report.spans()[0].span_context().trace_id() == 123);
  }

  SECTION("We can serialize the largest header") {
    REQUIRE(WriteReportRequestSpansHeader(
                &header_serialization[0], header_serialization.size(),
                std::numeric_limits<uint32_t>::max()) ==
            header_serialization.size());
  }
}
