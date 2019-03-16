#include "recorder/stream_recorder/host_header.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("HostHeader") {
  LightStepTracerOptions options;
  options.satellite_endpoints = {{"abc", 123}, {"abc123", 456}};
  HostHeader host_header{options};
  const char* header = static_cast<char*>(host_header.fragment().first);

  host_header.set_host("abc");
  REQUIRE(std::string{header} == "Host:   abc\r\n");

  host_header.set_host("abc123");
  REQUIRE(std::string{header} == "Host:abc123\r\n");
}
