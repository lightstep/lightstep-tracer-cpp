#pragma once

#include <cstdint>

#include "test/child_process_handle.h"
#include "test/http_connection.h"
#include "test/mock_satellite/mock_satellite.pb.h"

namespace lightstep {
class MockSatelliteHandle {
 public:
  explicit MockSatelliteHandle(uint16_t port);

  std::vector<collector::Span> spans();

  std::vector<collector::ReportRequest> reports();

 private:
  ChildProcessHandle handle_;
  HttpConnection connection_;
};
}  // namespace lightstep
