#include "tracer/propagation/trace_context.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("We can parse a trace-context") {
  TraceContext trace_context;

  SECTION("Parse an example trace context") {
    REQUIRE(ParseTraceContext(
        "00-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01",
        trace_context));
    REQUIRE(trace_context.version == 0);
    REQUIRE(trace_context.trace_id_high == 0x0af7651916cd43dd);
    REQUIRE(trace_context.trace_id_low == 0x8448eb211c80319c);
    REQUIRE(trace_context.parent_id == 0xb9c7c989f97918e1);
    REQUIRE(trace_context.trace_flags == 1);
  }
}
