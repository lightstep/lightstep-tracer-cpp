#pragma once

#include <cstdint>
#include <functional>

#include "lightstep/tracer.h"
#include "network/dns_resolver.h"
#include "recorder/stream_recorder/satellite_dns_resolution_manager.h"

namespace lightstep {
/**
 * Manages the resolution and assignment of satellite endpoints.
 */
class SatelliteEndpointManager {
  struct SatelliteHostManager {
    std::unique_ptr<SatelliteDnsResolutionManager> ipv4_resolutions;
    std::unique_ptr<SatelliteDnsResolutionManager> ipv6_resolutions;
    uint32_t address_index{0};
  };

 public:
  SatelliteEndpointManager(Logger& logger, EventBase& event_base,
                           const LightStepTracerOptions& tracer_options,
                           const StreamRecorderOptions& recorder_options,
                           std::function<void()> on_ready_callback);

  SatelliteEndpointManager(const SatelliteEndpointManager&) = delete;
  SatelliteEndpointManager(SatelliteEndpointManager&&) = delete;

  ~SatelliteEndpointManager() noexcept = default;

  SatelliteEndpointManager& operator=(const SatelliteEndpointManager&) = delete;
  SatelliteEndpointManager& operator=(SatelliteEndpointManager&&) = delete;

  /**
   * Assigns satellite endpoints using round robin.
   * @return a satellite endpoint.
   */
  IpAddress RequestEndpoint() noexcept;

 private:
  std::function<void()> on_ready_callback_;
  std::vector<SatelliteHostManager> host_managers_;
  std::unique_ptr<DnsResolver> dns_resolver_;
  std::vector<std::pair<int, uint16_t>> endpoints_;
  uint32_t endpoint_index_{0};
  int num_resolutions_ready_{0};

  void OnResolutionReady() noexcept;
};
}  // namespace lightstep
