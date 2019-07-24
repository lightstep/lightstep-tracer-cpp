#include "network/ares_dns_resolver/ares_dns_resolver.h"

#include <stdexcept>

#include "network/ares_dns_resolver/ares_library_handle.h"

#include <ares.h>
#include <event2/event.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// AresDnsResolution
//--------------------------------------------------------------------------------------------------
namespace {
class AresDnsResolution final : public DnsResolution {
 public:
  explicit AresDnsResolution(const hostent& hosts) noexcept : hosts_{hosts} {}

  // DnsResolution
  bool ForeachIpAddress(
      const FunctionRef<bool(const IpAddress& ip_address)>& f) const override {
    for (int i = 0; hosts_.h_addr_list[i] != nullptr; ++i) {
      IpAddress ip_address;
      if (hosts_.h_addrtype == AF_INET) {
        ip_address =
            IpAddress{*reinterpret_cast<in_addr*>(hosts_.h_addr_list[i])};
      } else if (hosts_.h_addrtype == AF_INET6) {
        ip_address =
            IpAddress{*reinterpret_cast<in6_addr*>(hosts_.h_addr_list[i])};
      } else {
        continue;
      }
      if (!f(ip_address)) {
        return false;
      }
    }
    return true;
  }

 private:
  const hostent& hosts_;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// OnDnsResolution
//--------------------------------------------------------------------------------------------------
static void OnDnsResolution(void* context, int status, int /*timeouts*/,
                            hostent* hosts) noexcept {
  auto callback = static_cast<DnsResolutionCallback*>(context);
  if (status != ARES_SUCCESS) {
    return callback->OnDnsResolution(DnsResolution{}, ares_strerror(status));
  }
  if (hosts == nullptr) {
    return callback->OnDnsResolution(DnsResolution{}, {});
  }
  AresDnsResolution resolution{*hosts};
  callback->OnDnsResolution(resolution, {});
}

//--------------------------------------------------------------------------------------------------
// SetAresOptions
//--------------------------------------------------------------------------------------------------
static void SetAresOptions(const DnsResolverOptions& resolver_options,
                           ares_options& options, int& option_mask) {
  options.udp_port = resolver_options.resolution_server_port;
  option_mask |= ARES_OPT_UDP_PORT;
  options.tcp_port = resolver_options.resolution_server_port;
  option_mask |= ARES_OPT_TCP_PORT;

  if (resolver_options.timeout != std::chrono::milliseconds{0}) {
    options.timeout = resolver_options.timeout.count();
    option_mask |= ARES_OPT_TIMEOUTMS;
  }

  if (resolver_options.resolution_servers.empty()) {
    return;
  }

  options.servers =
      const_cast<in_addr*>(resolver_options.resolution_servers.data());
  options.nservers =
      static_cast<int>(resolver_options.resolution_servers.size());
  option_mask |= ARES_OPT_SERVERS;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
AresDnsResolver::AresDnsResolver(Logger& logger, EventBase& event_base,
                                 const DnsResolverOptions& resolver_options)
    : logger_{logger},
      event_base_{event_base},
      timer_{event_base,
             MakeTimerCallback<AresDnsResolver, &AresDnsResolver::OnTimeout>(),
             static_cast<void*>(this)} {
  if (ares_library_handle_ == nullptr) {
    throw std::runtime_error{"ares failed to initialize"};
  }
  struct ares_options options = {};
  options.sock_state_cb = [](void* context, ares_socket_t file_descriptor,
                             int read, int write) noexcept {
    static_cast<AresDnsResolver*>(context)->OnSocketStateChange(file_descriptor,
                                                                read, write);
  };
  options.sock_state_cb_data = static_cast<void*>(this);
  int option_mask = 0;
  option_mask |= ARES_OPT_SOCK_STATE_CB;
  SetAresOptions(resolver_options, options, option_mask);
  auto status = ares_init_options(&channel_, &options, option_mask);
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
void AresDnsResolver::Resolve(const char* name, int family,
                              DnsResolutionCallback& callback) noexcept {
  ares_gethostbyname(channel_, name, family, OnDnsResolution,
                     static_cast<void*>(&callback));
}

//--------------------------------------------------------------------------------------------------
// OnSocketStateChange
//--------------------------------------------------------------------------------------------------
void AresDnsResolver::OnSocketStateChange(FileDescriptor file_descriptor,
                                          int read, int write) noexcept try {
  if (read == 0 && write == 0) {
    socket_events_.erase(file_descriptor);
    return;
  }
  short options = 0;
  if (read == 1) {
    options |= EV_READ;
  }
  if (write == 1) {
    options |= EV_WRITE;
  }
  Event event{event_base_, file_descriptor, options,
              MakeEventCallback<AresDnsResolver, &AresDnsResolver::OnEvent>(),
              static_cast<void*>(this)};
  event.Add(nullptr);
  socket_events_[file_descriptor] = std::move(event);
} catch (const std::exception& e) {
  logger_.Error("failed to set up ares socket events: ", e.what());
}

//--------------------------------------------------------------------------------------------------
// OnEvent
//--------------------------------------------------------------------------------------------------
void AresDnsResolver::OnEvent(FileDescriptor file_descriptor,
                              short what) noexcept try {
  auto read_file_descriptor =
      (what & EV_READ) != 0 ? file_descriptor : ARES_SOCKET_BAD;
  auto write_file_descriptor =
      (what & EV_WRITE) != 0 ? file_descriptor : ARES_SOCKET_BAD;
  ares_process_fd(channel_, read_file_descriptor, write_file_descriptor);
  UpdateTimer();
} catch (const std::exception& e) {
  logger_.Error("failed to process ares event: ", e.what());
}

//--------------------------------------------------------------------------------------------------
// OnTimeout
//--------------------------------------------------------------------------------------------------
void AresDnsResolver::OnTimeout() noexcept { OnEvent(ARES_SOCKET_BAD, 0); }

//--------------------------------------------------------------------------------------------------
// UpdateTimer
//--------------------------------------------------------------------------------------------------
void AresDnsResolver::UpdateTimer() {
  timeval timeout;
  auto tv = ares_timeout(channel_, nullptr, &timeout);
  timer_.Reset(tv);
}

//--------------------------------------------------------------------------------------------------
// MakeDnsResolver
//--------------------------------------------------------------------------------------------------
std::unique_ptr<DnsResolver> MakeDnsResolver(
    Logger& logger, EventBase& event_base, const DnsResolverOptions& options) {
  return std::unique_ptr<DnsResolver>{
      new AresDnsResolver{logger, event_base, options}};
}
}  // namespace lightstep
