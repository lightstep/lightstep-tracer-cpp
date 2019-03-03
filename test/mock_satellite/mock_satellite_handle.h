#pragma once

#include <cstdint>

#include "test/child_process_handle.h"
#include "test/mock_satellite/mock_satellite.pb.h"
#include "test/http_connection.h"

namespace lightstep {
class MockSatelliteHandle {
 public:
  MockSatelliteHandle() noexcept = default;

  explicit MockSatelliteHandle(uint16_t port);

  std::vector<collector::Span> spans();
 private:
  ChildProcessHandle handle_;
  HttpConnection connection_;
};
}  // namespace lightstep
