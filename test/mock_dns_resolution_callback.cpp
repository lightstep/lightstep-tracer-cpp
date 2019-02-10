#include "test/mock_dns_resolution_callback.h"

namespace lightstep {
void MockDnsResolutionCallback::OnDnsResolution(
    const DnsResolution& resolution,
    opentracing::string_view error_message) noexcept {
  resolution.ForeachIpAddress([&](const IpAddress& ip_address) {
    ip_addresses_.push_back(ip_address);
    return true;
  });
  error_message_ = error_message;
  event_base_.LoopBreak();
}
}  // namespace lightstep
