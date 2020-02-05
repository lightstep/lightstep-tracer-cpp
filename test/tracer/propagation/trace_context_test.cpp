#include "tracer/propagation/trace_context.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("We can serialize and deserialize a trace-context") {
  TraceContext trace_context;

  opentracing::string_view valid_trace_context =
      "00-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01";

  SECTION("We can parse a valid trace context") {
    REQUIRE(ParseTraceContext(valid_trace_context, trace_context));
    REQUIRE(trace_context.version == 0);
    REQUIRE(trace_context.trace_id_high == 0x0af7651916cd43dd);
    REQUIRE(trace_context.trace_id_low == 0x8448eb211c80319c);
    REQUIRE(trace_context.parent_id == 0xb9c7c989f97918e1);
    REQUIRE(trace_context.trace_flags == 1);
  }

  SECTION("We fail if we encounter an invalid character") {
    for (int i = 0; i < static_cast<int>(valid_trace_context.size()); ++i) {
      std::string s = valid_trace_context;
      s[i] = 'X';
      REQUIRE(!ParseTraceContext(s, trace_context));
    }
  }

  SECTION(
      "We fail when trace context has fewer than the minimum required "
      "characters") {
    std::string s = valid_trace_context;
    s = s.substr(1);
    REQUIRE(!ParseTraceContext(s, trace_context));
  }

  SECTION(
      "We can parse a trace-context with more than the required characters") {
    std::string s = valid_trace_context;
    s += ' ';
    REQUIRE(ParseTraceContext(s, trace_context));
  }

  SECTION(
      "If we deserialize a trace-context and serialize again, we'll get the "
      "same value") {
    REQUIRE(ParseTraceContext(valid_trace_context, trace_context));
    std::string s(TraceContextLength, ' ');
    SerializeTraceContext(trace_context, &s[0]);
    REQUIRE(s == valid_trace_context);
  }
}
