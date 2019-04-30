#include "recorder/stream_recorder/satellite_dns_resolution_manager.h"

#include "3rd_party/catch2/catch.hpp"
#include "network/dns_resolver.h"
#include "test/mock_dns_server/mock_dns_server_handle.h"
#include "test/ports.h"
using namespace lightstep;

const auto DnsRefreshPeriod =
    std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::milliseconds{100});

const auto DnsFailureRetryPeriod =
    std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::milliseconds{100});

const auto ResolutionTimeout = std::chrono::milliseconds{100};

TEST_CASE("SatelliteDnsResolutionManager") {
  MockDnsServerHandle dns_server{static_cast<uint16_t>(
      PortAssignments::SatelliteDnsResolutionManagerTest)};

  StreamRecorderOptions recorder_options;
  recorder_options.min_dns_resolution_refresh_period = DnsRefreshPeriod;
  recorder_options.max_dns_resolution_refresh_period = DnsRefreshPeriod;
  recorder_options.dns_failure_retry_period = DnsFailureRetryPeriod;
  auto& resolver_options = recorder_options.dns_resolver_options;
  resolver_options.resolution_server_port =
      static_cast<uint16_t>(PortAssignments::SatelliteDnsResolutionManagerTest);
  resolver_options.resolution_servers = {
      IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  resolver_options.timeout = ResolutionTimeout;
  Logger logger;
  EventBase event_base;

  std::function<void()> on_ready_callback = [&] { event_base.LoopBreak(); };

  SECTION("Hosts get resolved to ip addresses.") {
    SatelliteDnsResolutionManager resolution_manager{
        logger,  event_base,     recorder_options,
        AF_INET, "test.service", on_ready_callback};
    resolution_manager.Start();
    event_base.Dispatch();
    REQUIRE(resolution_manager.ip_addresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.2"}});
  }

  SECTION("Dns resolutions are periodically refreshed.") {
    SatelliteDnsResolutionManager resolution_manager{
        logger,  event_base,     recorder_options,
        AF_INET, "flip.service", on_ready_callback};
    resolution_manager.Start();
    event_base.Dispatch();
    REQUIRE(resolution_manager.ip_addresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.2"}});
    event_base.OnTimeout(
        1.5 * DnsRefreshPeriod,
        [](int /*socket*/, short /*what*/, void* context) {
          static_cast<EventBase*>(context)->LoopBreak();
        },
        static_cast<void*>(&event_base));
    event_base.Dispatch();
    REQUIRE(resolution_manager.ip_addresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.3"}});
  }

  SECTION("Dns resolutions are retried when if there's an error.") {
    SatelliteDnsResolutionManager resolution_manager{
        logger,  event_base,      recorder_options,
        AF_INET, "flaky.service", on_ready_callback};
    resolution_manager.Start();
    event_base.Dispatch();
    REQUIRE(resolution_manager.ip_addresses() ==
            std::vector<IpAddress>{IpAddress{"192.168.0.1"}});
  }
}
