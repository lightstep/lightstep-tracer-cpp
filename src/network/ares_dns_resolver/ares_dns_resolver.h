#pragma once

#include "network/dns_resolver.h"

struct ares_channeldata;

namespace lightstep {
class AresDnsResolver : public DnsResolver {
 public:
  AresDnsResolver(Logger& logger, EventBase& event_base,
                  DnsResolverOptions&& options);

  ~AresDnsResolver() noexcept override;

  // DnsResolver
  void Resolve(const char* host, ResolverCallback callback,
               void* context) noexcept override;

 private:
  Logger& logger_;
  EventBase& event_base_;
  ares_channeldata* channel_;

  static int SocketConfigCallback(int file_descriptor, int type,
                                  void* context) noexcept;
};
}  // namespace lightstep
