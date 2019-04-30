#include "test/echo_server/echo_server_handle.h"

#include <string>

#include "test/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
EchoServerHandle::EchoServerHandle(uint16_t http_port, uint16_t tcp_port)
    : handle_{"test/echo_server/echo_server",
              {std::to_string(http_port), std::to_string(tcp_port)}},
      http_connection_{"127.0.0.1", http_port} {
  if (!IsEventuallyTrue([http_port, tcp_port] {
        return CanConnect(http_port) && CanConnect(tcp_port);
      })) {
    std::cerr << "Failed to connect to echo server\n";
    std::terminate();
  }
}

//--------------------------------------------------------------------------------------------------
// data
//--------------------------------------------------------------------------------------------------
std::string EchoServerHandle::data() { return http_connection_.Get("/data"); }
}  // namespace lightstep
