#pragma once

#include <cstdint>
#include <functional>

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
                           const StreamRecorderOptions& recorder_options,
                           std::function<void()> on_ready_callback);

  SatelliteEndpointManager(const SatelliteEndpointManager&) = delete;
  SatelliteEndpointManager(SatelliteEndpointManager&&) = delete;

  SatelliteEndpointManager& operator=(const SatelliteEndpointManager&) = delete;
  SatelliteEndpointManager& operator=(SatelliteEndpointManager&&) = delete;

  IpAddress RequestEndpoint() noexcept;

 private:
  std::unique_ptr<DnsResolver> dns_resolver_;
  std::function<void()> on_ready_callback_;
  std::vector<SatelliteHostManager> host_managers_;
  std::vector<std::pair<int, uint16_t>> endpoints_;
  uint32_t endpoint_index_{0};
  int num_resolutions_ready_{0};

  void OnResolutionReady() noexcept;
};
}  // namespace lightstep
