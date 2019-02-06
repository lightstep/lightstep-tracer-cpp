#include "test/mock_dns_server/mock_dns_server_handle.h"

#include <string>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
MockDnsServerHandle::MockDnsServerHandle(uint16_t port)
    : handle_{"test/mock_dns_server/mock_dns_server", {std::to_string(port)}} {}
}  // namespace lightstep
