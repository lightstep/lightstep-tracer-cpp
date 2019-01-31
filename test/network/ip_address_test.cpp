#include "network/ip_address.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

TEST_CASE("IpAddress") {
  IpAddress addr1{"192.168.0.1"}, addr2{"192.168.1.1"},
      addr3{"FE80:CD00:0000:03DE:1257:0000:211E:729C"},
      addr4{"FE80:CD00:0000:0CDE:1257:0000:211E:729C"};

  SECTION("Addresses can be converted to strings.") {
    REQUIRE(ToString(addr1) == "192.168.0.1");
    REQUIRE(ToString(addr3) == "fe80:cd00:0:3de:1257:0:211e:729c");
  }

  SECTION("Addresses can be compared.") {
    REQUIRE(addr1 == addr1);
    REQUIRE(addr1 != addr2);
    REQUIRE(addr1 != addr3);
    REQUIRE(addr3 == addr3);
    REQUIRE(addr3 != addr4);
  }

  SECTION("Addresses can be constructed from sockaddr.") {
    IpAddress addr5{addr1.addr()}, addr6{addr3.addr()};
    REQUIRE(addr5 == addr1);
    REQUIRE(addr6 == addr3);
  }

  SECTION("The port can be set.") {
    addr1.set_port(8080);
    REQUIRE(addr1.ipv4_address().sin_port == htons(8080));
    addr3.set_port(8080);
    REQUIRE(addr3.ipv6_address().sin6_port == htons(8080));
  }
}
