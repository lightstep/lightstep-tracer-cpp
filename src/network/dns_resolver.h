#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include "common/function_ref.h"
#include "common/logger.h"
#include "network/event_base.h"
#include "network/ip_address.h"

#include <opentracing/string_view.h>

namespace lightstep {
/**
 * Internal options that can be used to control the behavior of dns resolvers.
 */
struct DnsResolverOptions {
  std::vector<in_addr> resolution_servers;
  uint16_t resolution_server_port{53};
  std::chrono::milliseconds timeout{2000};
};

/**
 * The result of a dns resolution.
 */
class DnsResolution {
 public:
  DnsResolution() noexcept = default;
  DnsResolution(const DnsResolution&) noexcept = default;
  DnsResolution(DnsResolution&&) noexcept = default;

  virtual ~DnsResolution() noexcept = default;

  DnsResolution& operator=(const DnsResolution&) noexcept = default;
  DnsResolution& operator=(DnsResolution&&) noexcept = default;

  /**
   * Iterate over the ip addresses of a dns resolution.
   * @param f supplies a callback to invoke for each ip address. f can return
   * false to bail out of the iteration.
   * @return true if f returns true for each ip address; false, otherwise.
   */
  virtual bool ForeachIpAddress(
      const FunctionRef<bool(const IpAddress& ip_address)>& /*f*/) const {
    return true;
  }
};

/**
 * A callback to be invoked when an asynchronous dns resolution completes.
 */
class DnsResolutionCallback {
 public:
  DnsResolutionCallback() noexcept = default;
  DnsResolutionCallback(const DnsResolutionCallback&) noexcept = default;
  DnsResolutionCallback(DnsResolutionCallback&&) noexcept = default;

  virtual ~DnsResolutionCallback() noexcept = default;

  DnsResolutionCallback& operator=(const DnsResolutionCallback&) noexcept =
      default;
  DnsResolutionCallback& operator=(DnsResolutionCallback&&) noexcept = default;

  /**
   * The function invoked when dns resolution completes.
   * @param dns_resolution supplies the resolved ip addresses.
   * @param error_message supplies an error message if dns resolution failed.
   */
  virtual void OnDnsResolution(
      const DnsResolution& dns_resolution,
      opentracing::string_view error_message) noexcept = 0;
};

/**
 * Interface for an asynchronous dns resolver.
 */
class DnsResolver {
 public:
  DnsResolver() noexcept = default;
  DnsResolver(const DnsResolver&) = delete;
  DnsResolver(DnsResolver&&) = delete;

  virtual ~DnsResolver() noexcept = default;

  DnsResolver& operator=(const DnsResolver&) = delete;
  DnsResolver& operator=(DnsResolver&&) = delete;

  /**
   * Resolves a name and invokes the provided callback when complete.
   * @param name supplies the name to resolve.
   * @param family identifies whether to query for ipv4 or ipv6 addresses.
   * @param callback supplies the callback to invoke when resolution completes.
   */
  virtual void Resolve(const char* name, int family,
                       DnsResolutionCallback& callback) noexcept = 0;
};

/**
 * Interface to construct a DnsResolver.
 * @param logger supplies the place to write logs.
 * @param supplies the libevent dispatcher.
 * @param options sets the behavior of the dns resolver.
 * @return the dns resolver.
 */
std::unique_ptr<DnsResolver> MakeDnsResolver(Logger& logger,
                                             EventBase& event_base,
                                             const DnsResolverOptions& options);
}  // namespace lightstep
