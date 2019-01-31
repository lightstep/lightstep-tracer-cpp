#pragma once

#include <chrono>
#include <forward_list>
#include <vector>

#include "common/logger.h"
#include "network/event_base.h"
#include "network/ip_address.h"

namespace lightstep {
struct DnsResolverOptions {
  std::vector<IpAddress> resolution_servers;
  std::chrono::microseconds timeout;
};

class DnsResolver {
 public:
  DnsResolver() noexcept = default;
  DnsResolver(const DnsResolver&) = delete;
  DnsResolver(DnsResolver&&) = delete;

  virtual DnsResolver() noexcept = default;

  DnsResolver& operator=(const DnsResolver&) = delete;
  DnsResolver& operator=(DnsResolver&&) = delete;

  using ResolverCallback = void (*)(std::forward_list<IpAddress>&& addresses,
                                    void* context);

  virtual void Resolve(const char* host, ResolverCallback callback,
                       void* context) noexcept = 0;
};

std::unique_ptr<DnsResolver> MakeDnsResolver(Logger& logger,
                                             EventBase& event_base,
                                             DnsResolverOptions&& options);
}  // namespace lightstep
