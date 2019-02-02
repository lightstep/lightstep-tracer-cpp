#include "network/ares_dns_resolver/ares_dns_resolver.h"

#include <ares.h>
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
  struct ares_options options = {};
  int option_mask = 0;
  auto status = ares_init_options(&channel_, &options, option_mask);
  ares_set_socket_configure_callback(channel_, SocketConfigCallback,
                                     static_cast<void*>(this));
  if (status != ARES_SUCCESS) {
    logger_.Error("ares_init_options failed: ", ares_strerror(status));
    throw std::runtime_error{"ares_init_options failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
AresDnsResolver::~AresDnsResolver() noexcept { ares_destroy(channel_); }

//--------------------------------------------------------------------------------------------------
// Resolve
//--------------------------------------------------------------------------------------------------
void AresDnsResolver::Resolve(const char* /*host*/,
                              ResolverCallback /*callback*/,
                              void* /*context*/) noexcept {}

//--------------------------------------------------------------------------------------------------
// SocketConfigCallback
//--------------------------------------------------------------------------------------------------
int AresDnsResolver::SocketConfigCallback(int /*file_descriptor*/, int /*type*/,
                                          void* /*context*/) noexcept {
  return ARES_SUCCESS;
}
}  // namespace lightstep
