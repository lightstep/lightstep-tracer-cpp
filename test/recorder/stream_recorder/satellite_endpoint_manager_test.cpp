#include "recorder/stream_recorder/satellite_endpoint_manager.h"

#include "3rd_party/catch2/catch.hpp"
#include "network/dns_resolver.h"
#include "test/mock_dns_server/mock_dns_server_handle.h"
#include "test/ports.h"
using namespace lightstep;

const auto ResolutionTimeout = std::chrono::milliseconds{100};

TEST_CASE("SatelliteEndpointManager") {
  MockDnsServerHandle dns_server{
      static_cast<uint16_t>(PortAssignments::SatelliteEndpointManagerTest)};

  StreamRecorderOptions recorder_options;
  auto& resolver_options = recorder_options.dns_resolver_options;
  resolver_options.resolution_server_port =
      static_cast<uint16_t>(PortAssignments::SatelliteEndpointManagerTest);
  resolver_options.resolution_servers = {
      IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  resolver_options.timeout = ResolutionTimeout;
  LightStepTracerOptions tracer_options;
  Logger logger;
  EventBase event_base;
  std::function<void()> on_ready_callback = [&] { event_base.LoopBreak(); };

  SECTION(
      "Round robin is used when assigning endpoints if multiple IPs are "
      "available for a port.") {
    tracer_options.satellite_endpoints = {{"satellites.service", 1234}};
    auto name = tracer_options.satellite_endpoints[0].first.c_str();
    SatelliteEndpointManager endpoint_manager{logger, event_base,
                                              tracer_options, recorder_options,
                                              on_ready_callback};
    endpoint_manager.Start();
    event_base.Dispatch();

    REQUIRE(endpoint_manager.RequestEndpoint() ==
            std::make_pair(IpAddress{"192.168.0.1", 1234}, name));
    REQUIRE(endpoint_manager.RequestEndpoint() ==
            std::make_pair(IpAddress{"192.168.0.2", 1234}, name));
  }

  SECTION(
      "Round robin is used when assigning endpoints if multiple ports are "
      "listed for a host.") {
    tracer_options.satellite_endpoints = {{"satellites.service", 1234},
                                          {"satellites.service", 2345}};
    SatelliteEndpointManager endpoint_manager{logger, event_base,
                                              tracer_options, recorder_options,
                                              on_ready_callback};
    endpoint_manager.Start();
    event_base.Dispatch();
    REQUIRE(endpoint_manager.RequestEndpoint().first ==
            IpAddress{"192.168.0.1", 1234});
    REQUIRE(endpoint_manager.RequestEndpoint().first ==
            IpAddress{"192.168.0.2", 2345});
  }
}
