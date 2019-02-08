#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include "common/logger.h"
#include "network/event_base.h"
#include "network/ip_address.h"

#include <opentracing/string_view.h>

namespace lightstep {
struct DnsResolverOptions {
  std::vector<in_addr> resolution_servers;
  uint16_t resolution_server_port{53};
  std::chrono::milliseconds timeout;
};

class DnsResolution {
 public:
  DnsResolution() noexcept = default;
  DnsResolution(const DnsResolution&) noexcept = default;
  DnsResolution(DnsResolution&&) noexcept = default;

  virtual ~DnsResolution() noexcept = default;

  DnsResolution& operator=(const DnsResolution&) noexcept = default;
  DnsResolution& operator=(DnsResolution&&) noexcept = default;

  virtual bool ForeachIpAddress(
      std::function<bool(const IpAddress& ip_address)> /*f*/) const {
    return true;
  }
};

class DnsResolutionCallback {
 public:
  DnsResolutionCallback() noexcept = default;
  DnsResolutionCallback(const DnsResolutionCallback&) noexcept = default;
  DnsResolutionCallback(DnsResolutionCallback&&) noexcept = default;

  virtual ~DnsResolutionCallback() noexcept = default;

  DnsResolutionCallback& operator=(const DnsResolutionCallback&) noexcept =
      default;
  DnsResolutionCallback& operator=(DnsResolutionCallback&&) noexcept = default;

  virtual void OnDnsResolution(
      const DnsResolution& dns_resolution,
      opentracing::string_view error_message) noexcept = 0;
};

class DnsResolver {
 public:
  DnsResolver() noexcept = default;
  DnsResolver(const DnsResolver&) = delete;
  DnsResolver(DnsResolver&&) = delete;

  virtual ~DnsResolver() noexcept = default;

  DnsResolver& operator=(const DnsResolver&) = delete;
  DnsResolver& operator=(DnsResolver&&) = delete;

  virtual void Resolve(const char* name, int family,
                       DnsResolutionCallback& callback) noexcept = 0;
};

std::unique_ptr<DnsResolver> MakeDnsResolver(Logger& logger,
                                             EventBase& event_base,
                                             DnsResolverOptions&& options);
}  // namespace lightstep
