#include "recorder/stream_recorder/utility.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SeparateEndpoints") {
  std::vector<const char*> hosts;
  std::vector<std::pair<int, uint16_t>> indexed_endpoints;

  SECTION("Endpoints with the same host are separated out.") {
    std::tie(hosts, indexed_endpoints) =
        SeparateEndpoints({{"abc", 123}, {"xyz", 456}, {"abc", 789}});
    REQUIRE(hosts.size() == 2);
    REQUIRE(hosts[0] == std::string{"abc"});
    REQUIRE(hosts[1] == std::string{"xyz"});
    REQUIRE(indexed_endpoints == std::vector<std::pair<int, uint16_t>>{
                                     {0, 123}, {1, 456}, {0, 789}});
  }
}
