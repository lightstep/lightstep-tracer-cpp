#include <iostream>
#include <thread>

#include "network/ares_dns_resolver/ares_dns_resolver.h"
#include "network/event_base.h"
#include "test/mock_dns_resolution_callback.h"
#include "test/mock_dns_server/mock_dns_server_handle.h"
#include "test/ports.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

const auto ResolutionTimeout = std::chrono::milliseconds{100};

TEST_CASE("AresDnsResolver") {
  Logger logger;
  EventBase event_base;
  DnsResolverOptions options;
  options.timeout = ResolutionTimeout;
  MockDnsServerHandle dns_server{
      static_cast<uint16_t>(PortAssignments::AresDnsResolverTest)};
  options.resolution_server_port =
      static_cast<uint16_t>(PortAssignments::AresDnsResolverTest);
  options.resolution_servers = {IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  auto resolver = MakeDnsResolver(logger, event_base, options);
  MockDnsResolutionCallback callback{event_base};

  SECTION("We can resolve ipv4 addresses.") {
    resolver->Resolve("test.service", AF_INET, callback);
    event_base.Dispatch();
    REQUIRE(callback.ip_addresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.2"}});
  }

  SECTION("We can resolve ipv6 addresses.") {
    resolver->Resolve("ipv6.service", AF_INET6, callback);
    event_base.Dispatch();
    REQUIRE(callback.ip_addresses() ==
            std::vector<IpAddress>{
                IpAddress{"2001:0db8:85a3:0000:0000:8a2e:0370:7334"}});
  }

  SECTION("If no answer is received, then the query times out.") {
    resolver->Resolve("timeout.service", AF_INET, callback);
    event_base.Dispatch();
    REQUIRE(callback.ip_addresses().empty());
    REQUIRE(!callback.error_message().empty());
  }

  SECTION("The dns resolver doesn't block the process from exiting.") {
    event_base.OnTimeout(
        ResolutionTimeout / 4,
        [](int /*file_descriptor*/, short /*what*/, void* context) {
          static_cast<EventBase*>(context)->LoopBreak();
        },
        static_cast<void*>(&event_base));
    resolver->Resolve("timeout.service", AF_INET, callback);
    auto t1 = std::chrono::system_clock::now();
    event_base.Dispatch();
    auto t2 = std::chrono::system_clock::now();
    REQUIRE((t2 - t1) < ResolutionTimeout / 2);
    resolver.reset();
  }
}
