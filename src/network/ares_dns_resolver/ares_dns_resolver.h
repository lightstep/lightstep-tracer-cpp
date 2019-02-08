#pragma once

#include <unordered_map>

#include "network/dns_resolver.h"
#include "network/event.h"
#include "network/timer_event.h"

struct ares_channeldata;

namespace lightstep {
/**
 * A DnsResolver using the c-ares library.
 */
class AresDnsResolver final : public DnsResolver {
 public:
  AresDnsResolver(Logger& logger, EventBase& event_base,
                  DnsResolverOptions&& resolver_options);

  AresDnsResolver(const AresDnsResolver&) = delete;
  AresDnsResolver(AresDnsResolver&&) = delete;

  ~AresDnsResolver() noexcept override;

  AresDnsResolver& operator=(const AresDnsResolver&) = delete;
  AresDnsResolver& operator=(AresDnsResolver&&) = delete;

  // DnsResolver
  void Resolve(const char* name, int family,
               DnsResolutionCallback& callback) noexcept override;

 private:
  Logger& logger_;
  EventBase& event_base_;
  ares_channeldata* channel_;

  std::unordered_map<int, Event> socket_events_;
  TimerEvent timer_;

  void OnSocketStateChange(int file_descriptor, int read, int write) noexcept;

  void OnEvent(int file_descriptor, short what) noexcept;

  void OnTimeout() noexcept;

  void UpdateTimer();
};
}  // namespace lightstep
