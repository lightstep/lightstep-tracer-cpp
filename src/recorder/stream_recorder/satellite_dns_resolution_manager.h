#pragma once

#include <string>
#include <vector>

#include "common/logger.h"
#include "network/dns_resolver.h"
#include "network/event_base.h"
#include "network/ip_address.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
class SatelliteDnsResolutionManager final : public DnsResolutionCallback {
 public:
  SatelliteDnsResolutionManager(Logger& logger, EventBase& event_base,
                                DnsResolver& dns_resolver,
                                const StreamRecorderOptions& recorder_options,
                                int family, const char* name);

  const std::vector<IpAddress>& ip_addresses() const noexcept {
    return ip_addresses_;
  }

  // DnsResolutionCallback
  void OnDnsResolution(
      const DnsResolution& dns_resolution,
      opentracing::string_view error_message) noexcept override;

 private:
  Logger& logger_;
  EventBase& event_base_;
  DnsResolver& dns_resolver_;
  const StreamRecorderOptions& recorder_options_;
  int family_;
  const char* name_;

  std::vector<IpAddress> ip_addresses_;

  void OnRefresh() noexcept;

  void OnFailure() noexcept;

  void ScheduleRefresh() noexcept;
};
}  // namespace lightstep
