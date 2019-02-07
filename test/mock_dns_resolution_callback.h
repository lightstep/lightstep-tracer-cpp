#pragma once

#include "network/dns_resolver.h"
#include "network/event_base.h"

namespace lightstep {
class MockDnsResolutionCallback final : public DnsResolutionCallback {
 public:
  explicit MockDnsResolutionCallback(EventBase& event_base)
      : event_base_{event_base} {}

  void OnDnsResolution(
      const DnsResolution& resolution,
      opentracing::string_view error_message) noexcept override;

  const std::vector<IpAddress> ipAddresses() const noexcept {
    return ip_addresses_;
  }

  const std::string& errorMessage() const noexcept { return error_message_; }

 private:
  EventBase& event_base_;
  std::string error_message_;
  std::vector<IpAddress> ip_addresses_;
};
}  // namespace lightstep
