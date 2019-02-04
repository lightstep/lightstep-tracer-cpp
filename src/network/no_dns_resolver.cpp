#include "network/dns_resolver.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// SingleIpDnsResolution
//--------------------------------------------------------------------------------------------------
namespace {
class SingleIpDnsResolution : public DnsResolution {
 public:
  explicit SingleIpDnsResolution(const IpAddress& ip_address) noexcept
      : ip_address_{ip_address} {}

  bool forEachIpAddress(
      std::function<bool(const IpAddress& ip_address)> f) const override {
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
class IpOnlyDnsResolver : public DnsResolver {
 public:
  explicit IpOnlyDnsResolver(Logger& logger) noexcept : logger_{logger} {}

  void Resolve(const char* name,
               const DnsResolutionCallback& callback) noexcept override {
    IpAddress ip_address;
    try {
      ip_address = IpAddress{name};
    } catch (const std::exception& /*e*/) {
      logger_.Error("Failed to resolve ", name,
                    " to an ip address: an explicit ip must be provided or the "
                    "tracer needs to be rebuilt with c-ares support.");
      return callback.OnDnsResolution(DnsResolution{}, {});
    }
    callback.OnDnsResolution(SingleIpDnsResolution{ip_address}, {});
  }

 private:
  Logger& logger_;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// MakeDnsResolver
//--------------------------------------------------------------------------------------------------
std::unique_ptr<DnsResolver> MakeDnsResolver(Logger& logger,
                                             EventBase& /*event_base*/,
                                             DnsResolverOptions&& /*options*/) {
  return std::unique_ptr<DnsResolver>{new IpOnlyDnsResolver{logger}};
}
}  // namespace lightstep
