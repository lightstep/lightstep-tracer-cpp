#include "network/ares_dns_resolver/ares_dns_resolver.h"

#include <stdexcept>

#include "network/ares_dns_resolver/ares_library_handle.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
AresDnsResolver::AresDnsResolver(Logger& logger, EventBase& event_base,
                                 DnsResolverOptions&& /*options*/)
    : logger_{logger}, event_base_{event_base} {
  if (!AresLibraryHandle::Instance.initialized()) {
    throw std::runtime_error{"ares not initialized"};
  }
}

//--------------------------------------------------------------------------------------------------
// Resolve
//--------------------------------------------------------------------------------------------------
void AresDnsResolver::Resolve(const char* /*host*/,
                              ResolverCallback /*callback*/,
                              void* /*context*/) noexcept {}
}  // namespace lightstep
