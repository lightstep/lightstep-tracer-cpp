#include "recorder/stream_recorder/utility.h"

#include "3rd_party/catch2/catch.hpp"
#include "lightstep-tracer-common/collector.pb.h"
using namespace lightstep;

TEST_CASE("SeparateEndpoints") {
  std::vector<const char*> hosts;
  std::vector<std::pair<int, uint16_t>> indexed_endpoints;

  SECTION("Endpoints with the same host are separated out.") {
    std::vector<std::pair<std::string, uint16_t>> endpoints = {
        {"abc", 123}, {"xyz", 456}, {"abc", 789}};

    std::tie(hosts, indexed_endpoints) = SeparateEndpoints(endpoints);
    REQUIRE(hosts.size() == 2);
    REQUIRE(hosts[0] == std::string{"abc"});
    REQUIRE(hosts[1] == std::string{"xyz"});
    REQUIRE(indexed_endpoints == std::vector<std::pair<int, uint16_t>>{
                                     {0, 123}, {1, 456}, {0, 789}});
  }

  SECTION("Case is ignored when comparing hosts.") {
    std::vector<std::pair<std::string, uint16_t>> endpoints = {{"abc", 123},
                                                               {"ABC", 456}};
    std::tie(hosts, indexed_endpoints) = SeparateEndpoints(endpoints);
    REQUIRE(hosts.size() == 1);
  }
}

TEST_CASE("WriteStreamHeaderCommonFragment") {
  LightStepTracerOptions tracer_options;
  tracer_options.access_token = "abc123";
  tracer_options.tags = {{"xyz", 456}};
  auto serialization = WriteStreamHeaderCommonFragment(tracer_options, 789);
  collector::ReportRequest report;
  REQUIRE(report.ParseFromString(serialization));
  REQUIRE(report.auth().access_token() == "abc123");
  REQUIRE(report.reporter().reporter_id() == 789);
  auto& tags = report.reporter().tags();
  REQUIRE(tags.size() == 1);
  REQUIRE(tags[0].key() == "xyz");
  REQUIRE(tags[0].int_value() == 456);
}

TEST_CASE("Contains") {
  const char* s = "abc123";
  REQUIRE(Contains(s, 3, s + 1));
  REQUIRE(!Contains(s, 3, s + 3));
}
