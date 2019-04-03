#include "recorder/stream_recorder/status_line_parser.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("StatusLineParser") {
  StatusLineParser parser;
  REQUIRE(!parser.completed());

  SECTION("The status line is correctly parsed out of an http response.") {
    parser.Parse("HTTP/1.1 200 OK\r\n");
    REQUIRE(parser.completed());
    REQUIRE(parser.status_code() == 200);
    REQUIRE(parser.reason() == "OK");
  }

  SECTION("The status line can be parsed in multiple pieces.") {
    parser.Parse("HTTP/");
    REQUIRE(!parser.completed());
    parser.Parse("1.1 200 OK\r\n");
    REQUIRE(parser.completed());
    parser.Parse("ignored");
    REQUIRE(parser.completed());
    REQUIRE(parser.status_code() == 200);
    REQUIRE(parser.reason() == "OK");
  }

  SECTION("The parser can be reset.") {
    parser.Parse("HTTP/1.1 200 OK\r\n");
    REQUIRE(parser.completed());
    parser.Reset();
    REQUIRE(!parser.completed());
    parser.Parse("HTTP/1.1 404 Deadly failure\r\n");
    REQUIRE(parser.completed());
    REQUIRE(parser.status_code() == 404);
    REQUIRE(parser.reason() == "Deadly failure");
  }

  SECTION("The parser throws exceptions when given an invalid status line.") {
    REQUIRE_THROWS(parser.Parse("abc\r\n"));
    REQUIRE_THROWS(parser.Parse(" abc\r\n"));
  }
}
