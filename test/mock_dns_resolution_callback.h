#pragma once

#include "network/dns_resolver.h"
#include "network/event_base.h"

namespace lightstep {
/**
 * A dns resolution callback that captures everything provided.
 */
class MockDnsResolutionCallback final : public DnsResolutionCallback {
 public:
  explicit MockDnsResolutionCallback(EventBase& event_base)
      : event_base_{event_base} {}

  /**
   * @return the ip addresses provided by the dns resolution.
   */
  const std::vector<IpAddress> ip_addresses() const noexcept {
    return ip_addresses_;
  }

  /**
   * @return the error messages provied by the dns resolution.
   */
  const std::string& error_message() const noexcept { return error_message_; }

  // DnsResolutionCallback
  void OnDnsResolution(
      const DnsResolution& resolution,
      opentracing::string_view error_message) noexcept override;

 private:
  EventBase& event_base_;
  std::string error_message_;
  std::vector<IpAddress> ip_addresses_;
};
}  // namespace lightstep
