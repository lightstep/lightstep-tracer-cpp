#include "recorder/stream_recorder/satellite_dns_resolution_manager.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteDnsResolutionManager::SatelliteDnsResolutionManager(
    Logger& logger, EventBase& event_base, DnsResolver& dns_resolver,
    const StreamRecorderOptions& recorder_options, int family,
    const std::string& name)
    : logger_{logger},
      event_base_{event_base},
      dns_resolver_{dns_resolver},
      recorder_options_{recorder_options},
      family_{family},
      name_{name} {
  dns_resolver_.Resolve(name_.c_str(), family_, *this);
}

//--------------------------------------------------------------------------------------------------
// OnDnsResolution
//--------------------------------------------------------------------------------------------------
void SatelliteDnsResolutionManager::OnDnsResolution(
    const DnsResolution& dns_resolution,
    opentracing::string_view error_message) noexcept try {
  if (!error_message.empty()) {
    logger_.Debug("Failed to resolve ", name_, ": ", error_message);
    return OnFailure();
  }
  std::vector<IpAddress> ip_addresses;
  ip_addresses.reserve(ip_addresses_.size());
  dns_resolution.ForeachIpAddress([&](const IpAddress& ip_address) {
    ip_addresses.push_back(ip_address);
    return true;
  });
  if (!ip_addresses.empty()) {
    ip_addresses_ = std::move(ip_addresses);
  }
  ScheduleReset();
} catch (const std::exception& e) {
  logger_.Error("OnDnsResolution failed: ", e.what());
  OnFailure();
}

//--------------------------------------------------------------------------------------------------
// On6Reset
//--------------------------------------------------------------------------------------------------
void SatelliteDnsResolutionManager::OnReset() noexcept {
  dns_resolver_.Resolve(name_.c_str(), family_, *this);
}

//--------------------------------------------------------------------------------------------------
// OnFailure
//--------------------------------------------------------------------------------------------------
void SatelliteDnsResolutionManager::OnFailure() noexcept {}

//--------------------------------------------------------------------------------------------------
// SetupReset
//--------------------------------------------------------------------------------------------------
void SatelliteDnsResolutionManager::ScheduleReset() noexcept {}
}  // namespace lightstep
