#pragma once

#include "network/dns_resolver.h"

#include <ares.h>

namespace lightstep {
class AresDnsResolver : public DnsResolver {
 public:
  AresDnsResolver(Logger& logger, EventBase& event_base,
                  DnsResolverOptions&& options);

  // DnsResolver
  void Resolve(const char* host, ResolverCallback callback,
               void* context) noexcept override;

 private:
  Logger& logger_;
  EventBase& event_base_;
  ares_channel channel_;
};
}  // namespace lightstep
