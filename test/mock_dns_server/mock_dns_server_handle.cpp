#include "test/mock_dns_server/mock_dns_server_handle.h"

#include "common/logger.h"
#include "network/ares_dns_resolver/ares_dns_resolver.h"
#include "test/mock_dns_resolution_callback.h"
#include "test/utility.h"

#include <exception>
#include <iostream>
#include <string>

#include <sys/socket.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// IsMockServerUp
//--------------------------------------------------------------------------------------------------
static bool IsMockServerUp(uint16_t port) {
  EventBase event_base;
  MockDnsResolutionCallback callback{event_base};
  DnsResolverOptions options;
  Logger logger;
  options.resolution_server_port = port;
  options.resolution_servers = {IpAddress{"127.0.0.1"}.ipv4_address().sin_addr};
  AresDnsResolver resolver{logger, event_base, std::move(options)};
  resolver.Resolve("test.service", AF_INET, callback);
  event_base.Dispatch();
  return callback.ip_addresses().size() == 1;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
MockDnsServerHandle::MockDnsServerHandle(uint16_t port)
    : handle_{"test/mock_dns_server/mock_dns_server", {std::to_string(port)}} {
  if (!IsEventuallyTrue([port] { return IsMockServerUp(port); })) {
    std::cerr << "Failed to connect to mock dns server\n";
    std::terminate();
  }
}
}  // namespace lightstep
