#pragma once

#include <functional>
#include <string>
#include <vector>

#include "common/logger.h"
#include "common/noncopyable.h"
#include "network/dns_resolver.h"
#include "network/event_base.h"
#include "network/ip_address.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
/**
 * Manages the DNS resolution of a host.
 */
class SatelliteDnsResolutionManager final : public DnsResolutionCallback,
                                            private Noncopyable {
 public:
  SatelliteDnsResolutionManager(Logger& logger, EventBase& event_base,
                                const StreamRecorderOptions& recorder_options,
                                int family, const char* name,
                                std::function<void()> on_ready_callback);

  /**
   * Start resolving the host an ip addresses.
   */
  void Start() noexcept;

  /**
   * @return the vector of IP addresses the host resolves to.
   */
  const std::vector<IpAddress>& ip_addresses() const noexcept {
    return ip_addresses_;
  }

  /**
   * @return the name associated with this resolution
   */
  const char* name() const noexcept { return name_; }

  // DnsResolutionCallback
  void OnDnsResolution(
      const DnsResolution& dns_resolution,
      opentracing::string_view error_message) noexcept override;

 private:
  Logger& logger_;
  EventBase& event_base_;
  std::unique_ptr<DnsResolver> dns_resolver_;
  const StreamRecorderOptions& recorder_options_;
  int family_;
  const char* name_;
  std::function<void()> on_ready_callback_;

  std::vector<IpAddress> ip_addresses_;

  void OnRefresh() noexcept;

  void OnFailure() noexcept;

  void ScheduleRefresh() noexcept;
};
}  // namespace lightstep
