#include "recorder/stream_recorder/satellite_endpoint_manager.h"

#include <stdexcept>

#include "recorder/stream_recorder/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteEndpointManager::SatelliteEndpointManager(
    Logger& logger, EventBase& event_base,
    const LightStepTracerOptions& tracer_options,
    const StreamRecorderOptions& recorder_options,
    std::function<void()> on_ready_callback)
    : on_ready_callback_{std::move(on_ready_callback)} {
  if (tracer_options.satellite_endpoints.empty()) {
    throw std::runtime_error{"no satellite endpoints provided"};
  }
  std::vector<const char*> hosts;
  std::tie(hosts, endpoints_) =
      SeparateEndpoints(tracer_options.satellite_endpoints);

  host_managers_.reserve(hosts.size());
  auto on_resolution_ready = [this] { this->OnResolutionReady(); };
  for (auto& name : hosts) {
    SatelliteHostManager host_manager;
    host_manager.ipv4_resolutions.reset(
        new SatelliteDnsResolutionManager{logger, event_base, recorder_options,
                                          AF_INET, name, on_resolution_ready});
    host_manager.ipv6_resolutions.reset(
        new SatelliteDnsResolutionManager{logger, event_base, recorder_options,
                                          AF_INET6, name, on_resolution_ready});
    host_managers_.emplace_back(std::move(host_manager));
  }
}

//--------------------------------------------------------------------------------------------------
// Start
//--------------------------------------------------------------------------------------------------
void SatelliteEndpointManager::Start() noexcept {
  for (auto& host_manager : host_managers_) {
    host_manager.ipv4_resolutions->Start();
    host_manager.ipv6_resolutions->Start();
  }
}

//--------------------------------------------------------------------------------------------------
// RequestEndpoint
//--------------------------------------------------------------------------------------------------
std::pair<IpAddress, const char*>
SatelliteEndpointManager::RequestEndpoint() noexcept {
  auto endpoint_index_start = endpoint_index_;
  (void)endpoint_index_start;
  while (true) {
    // RequestEndpoint shouldn't be called until at least one host name is
    // resolved.
    assert(endpoint_index_ != endpoint_index_start + endpoints_.size());

    int host_index;
    uint16_t port;
    std::tie(host_index, port) =
        endpoints_[endpoint_index_++ % endpoints_.size()];
    auto& host_manager = host_managers_[host_index];

    auto& ip_addresses = !host_manager.ipv4_resolutions->ip_addresses().empty()
                             ? host_manager.ipv4_resolutions->ip_addresses()
                             : host_manager.ipv6_resolutions->ip_addresses();
    if (ip_addresses.empty()) {
      continue;
    }
    auto result =
        ip_addresses[host_manager.address_index++ % ip_addresses.size()];
    result.set_port(port);
    return std::make_pair(result, host_manager.ipv4_resolutions->name());
  }
}

//--------------------------------------------------------------------------------------------------
// OnResolutionReady
//--------------------------------------------------------------------------------------------------
void SatelliteEndpointManager::OnResolutionReady() noexcept {
  ++num_resolutions_ready_;
  if (num_resolutions_ready_ == 1) {
    on_ready_callback_();
  }
}
}  // namespace lightstep
