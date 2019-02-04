#include <iostream>

#include "network/ares_dns_resolver/ares_dns_resolver.h"
#include "network/event_base.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

class TestDnsResolutionCallback : public DnsResolutionCallback {
 public:
  explicit TestDnsResolutionCallback(EventBase& event_base)
      : event_base_{event_base} {}

  void OnDnsResolution(const DnsResolution& resolution,
                       opentracing::string_view /*error_message*/) const
      noexcept override {
    resolution.forEachIpAddress([](const IpAddress& ip_address) {
      std::cout << "address = " << ip_address << "\n";
      return true;
    });
    event_base_.LoopBreak();
  }

 private:
  EventBase& event_base_;
};

TEST_CASE("AresDnsResolver") {
  Logger logger;
  EventBase event_base;
  DnsResolverOptions options;
  AresDnsResolver resolver{logger, event_base, std::move(options)};
  TestDnsResolutionCallback test_callback{event_base};
  resolver.Resolve("www.google.com", test_callback);
  event_base.Dispatch();
}
