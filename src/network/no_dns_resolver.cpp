#include "network/dns_resolver.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// SingleIpDnsResolution
//--------------------------------------------------------------------------------------------------
namespace {
class SingleIpDnsResolution final : public DnsResolution {
 public:
  explicit SingleIpDnsResolution(const IpAddress& ip_address) noexcept
      : ip_address_{ip_address} {}

  bool ForeachIpAddress(
      const FunctionRef<bool(const IpAddress& ip_address)>& f) const override {
    return f(ip_address_);
  }

 private:
  IpAddress ip_address_;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// IpOnlyDnsResolver
//--------------------------------------------------------------------------------------------------
namespace {
class IpOnlyDnsResolver final : public DnsResolver {
 public:
  explicit IpOnlyDnsResolver(Logger& logger) noexcept : logger_{logger} {}

  void Resolve(const char* name, int family,
               DnsResolutionCallback& callback) noexcept override {
    IpAddress ip_address;
    try {
      ip_address = IpAddress{name};
    } catch (const std::exception& /*e*/) {
      logger_.Error("Failed to resolve ", name,
                    " to an ip address: an explicit ip must be provided or the "
                    "tracer needs to be rebuilt with c-ares support.");
      return callback.OnDnsResolution(DnsResolution{}, {});
    }
    if (ip_address.family() == family) {
      callback.OnDnsResolution(SingleIpDnsResolution{ip_address}, {});
    } else {
      callback.OnDnsResolution(DnsResolution{}, {});
    }
  }

 private:
  Logger& logger_;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// MakeDnsResolver
//--------------------------------------------------------------------------------------------------
std::unique_ptr<DnsResolver> MakeDnsResolver(
    Logger& logger, EventBase& /*event_base*/,
    const DnsResolverOptions& /*options*/) {
  return std::unique_ptr<DnsResolver>{new IpOnlyDnsResolver{logger}};
}
}  // namespace lightstep
