#include "recorder/stream_recorder/satellite_dns_resolution_manager.h"

#include "3rd_party/catch2/catch.hpp"
#include "network/dns_resolver.h"
#include "test/mock_dns_server/mock_dns_server_handle.h"
#include "test/ports.h"
using namespace lightstep;

TEST_CASE("SatelliteDnsResolutionManager") {
  MockDnsServerHandle dns_server{static_cast<uint16_t>(
      PortAssignments::SatelliteDnsResolutionManagerTest)};

  StreamRecorderOptions recorder_options;
  recorder_options.dns_resolver_options.resolution_server_port =
      static_cast<uint16_t>(PortAssignments::SatelliteDnsResolutionManagerTest);
  recorder_options.dns_resolver_options.resolution_servers = {
      IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  Logger logger;
  EventBase event_base;

  auto resolver = MakeDnsResolver(logger, event_base,
                                  recorder_options.dns_resolver_options);

  std::function<void()> on_ready_callback = [&] { event_base.LoopBreak(); };

  SECTION("Hosts get resolved to ip addresses.") {
    SatelliteDnsResolutionManager resolution_manager{
        logger,  event_base,     *resolver,        recorder_options,
        AF_INET, "test.service", on_ready_callback};
    event_base.Dispatch();
    REQUIRE(resolution_manager.ip_addresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.2"}});
  }
}
