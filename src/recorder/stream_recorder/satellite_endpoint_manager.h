#pragma once

#include <cstdint>

#include "lightstep/tracer.h"
#include "network/dns_resolver.h"
#include "recorder/stream_recorder/satellite_dns_resolution_manager.h"

namespace lightstep {
class SatelliteEndpointManager {
  struct SatelliteHostManager {
    SatelliteDnsResolutionManager ipv4_resolutions;
    SatelliteDnsResolutionManager ipv6_resolutions;
    uint32_t address_index;
  };
  struct SatelliteEndpoint {
    int host_index;
    uint16_t port;
  };

 public:
  SatelliteEndpointManager(Logger& logger, EventBase& event_base,
                           const LightStepTracerOptions& tracer_options,
                           const StreamRecorderOptions& recorder_options);

  IpAddress RequestEndpoint() noexcept;

 private:
  std::unique_ptr<DnsResolver> dns_resolver_;
  std::vector<SatelliteHostManager> host_managers_;
  std::vector<std::pair<int, uint16_t>> endpoints_;
  uint32_t endpoint_index_{0};
};
}  // namespace lightstep
