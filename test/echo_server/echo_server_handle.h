#pragma once

#include <cstdint>

#include "test/child_process_handle.h"
#include "test/http_connection.h"

namespace lightstep {
class EchoServerHandle {
 public:
  EchoServerHandle(uint16_t http_port, uint16_t tcp_port);

  std::string data();

 private:
  ChildProcessHandle handle_;
  HttpConnection http_connection_;
};
}  // namespace lightstep
