#include <iostream>
#include <thread>

#include "network/ares_dns_resolver/ares_dns_resolver.h"
#include "network/event_base.h"
#include "test/mock_dns_server/mock_dns_server_handle.h"
#include "test/ports.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

class TestDnsResolutionCallback : public DnsResolutionCallback {
 public:
  explicit TestDnsResolutionCallback(EventBase& event_base)
      : event_base_{event_base} {}

  void OnDnsResolution(
      const DnsResolution& resolution,
      opentracing::string_view error_message) noexcept override {
    resolution.forEachIpAddress([&](const IpAddress& ip_address) {
      ip_addresses_.push_back(ip_address);
      return true;
    });
    error_message_ = error_message;
    event_base_.LoopBreak();
  }

  const std::vector<IpAddress> ipAddresses() const noexcept {
    return ip_addresses_;
  }

  const std::string& errorMessage() const noexcept { return error_message_; }

 private:
  EventBase& event_base_;
  std::string error_message_;
  std::vector<IpAddress> ip_addresses_;
};

TEST_CASE("AresDnsResolver") {
  Logger logger;
  EventBase event_base;
  DnsResolverOptions options;
  MockDnsServerHandle dns_server{
      static_cast<uint16_t>(PortAssignments::AresDnsResolverTest)};
  options.resolution_server_port =
      static_cast<uint16_t>(PortAssignments::AresDnsResolverTest);
  options.resolution_servers = {IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  AresDnsResolver resolver{logger, event_base, std::move(options)};
  TestDnsResolutionCallback test_callback{event_base};

  std::this_thread::sleep_for(std::chrono::seconds{1});

  SECTION("We can resolve ipv4 addresses.") {
    resolver.Resolve("test.service", AF_INET, test_callback);
    event_base.Dispatch();
    REQUIRE(test_callback.ipAddresses().size() == 1);
  }
}
