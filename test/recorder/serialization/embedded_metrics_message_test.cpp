#include "recorder/serialization/embedded_metrics_message.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("EmbeddedMetricsMessage") {
  EmbeddedMetricsMessage metrics_message;
  metrics_message.set_num_dropped_spans(3);
  REQUIRE(metrics_message.num_dropped_spans() == 3);
  auto fragment = metrics_message.MakeFragment();

  SECTION("The metrics fragment can be deserialized by protobuf.") {
    collector::ReportRequest report;
    REQUIRE(report.ParseFromArray(fragment.first,
                                  static_cast<int>(fragment.second)));
    REQUIRE(report.internal_metrics().counts().size() == 1);
    auto& dropped_span_count = report.internal_metrics().counts()[0];
    REQUIRE(dropped_span_count.name() == "spans.dropped");
    REQUIRE(dropped_span_count.int_value() == 3);
  }
}
