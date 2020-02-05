#include "tracer/utility.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("We can append the values of w3 trace-state values") {
  std::string trace_state;

  SECTION("We can append to an empty trace-state") {
    AppendTraceState(trace_state, "abc=123");
    REQUIRE(trace_state == "abc=123");
  }

  SECTION("We can append to a non-empty trace-state") {
    trace_state = "abc=123";
    AppendTraceState(trace_state, "xyz=456");
    REQUIRE(trace_state == "abc=123,xyz=456");
  }
}
