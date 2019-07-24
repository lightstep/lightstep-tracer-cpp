#pragma once

#include <unordered_map>

#include "common/noncopyable.h"
#include "network/ares_dns_resolver/ares_library_handle.h"
#include "network/dns_resolver.h"
#include "network/event.h"
#include "network/timer_event.h"

struct ares_channeldata;

namespace lightstep {
/**
 * A DnsResolver using the c-ares library.
 */
class AresDnsResolver final : public DnsResolver, private Noncopyable {
 public:
  AresDnsResolver(Logger& logger, EventBase& event_base,
                  const DnsResolverOptions& resolver_options);

  ~AresDnsResolver() noexcept override;

  // DnsResolver
  void Resolve(const char* name, int family,
               DnsResolutionCallback& callback) noexcept override;

 private:
  std::shared_ptr<const AresLibraryHandle> ares_library_handle_{
      AresLibraryHandle::Instance};
  Logger& logger_;
  EventBase& event_base_;
  ares_channeldata* channel_;

  std::unordered_map<FileDescriptor, Event> socket_events_;
  TimerEvent timer_;

  void OnSocketStateChange(FileDescriptor file_descriptor, int read,
                           int write) noexcept;

  void OnEvent(FileDescriptor file_descriptor, short what) noexcept;

  void OnTimeout() noexcept;

  void UpdateTimer();
};
}  // namespace lightstep
