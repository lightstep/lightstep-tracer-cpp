#include <iostream>
#include <thread>

#include "network/ares_dns_resolver/ares_dns_resolver.h"
#include "network/event_base.h"
#include "test/mock_dns_resolution_callback.h"
#include "test/mock_dns_server/mock_dns_server_handle.h"
#include "test/ports.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

TEST_CASE("AresDnsResolver") {
  Logger logger;
  EventBase event_base;
  DnsResolverOptions options;
  MockDnsServerHandle dns_server{
      static_cast<uint16_t>(PortAssignments::AresDnsResolverTest)};
  options.resolution_server_port =
      static_cast<uint16_t>(PortAssignments::AresDnsResolverTest);
  options.resolution_servers = {IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  AresDnsResolver resolver{logger, event_base, std::move(options)};
  MockDnsResolutionCallback callback{event_base};

  SECTION("We can resolve ipv4 addresses.") {
    resolver.Resolve("test.service", AF_INET, callback);
    event_base.Dispatch();
    REQUIRE(callback.ipAddresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.2"}});
  }

  SECTION("We can resolve ipv6 addresses.") {
    resolver.Resolve("ipv6.service", AF_INET6, callback);
    event_base.Dispatch();
    REQUIRE(callback.ipAddresses() ==
            std::vector<IpAddress>{
                IpAddress{"2001:0db8:85a3:0000:0000:8a2e:0370:7334"}});
  }
}
