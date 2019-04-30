#pragma once

#include <cstdint>

#include "test/child_process_handle.h"
#include "test/http_connection.h"
#include "test/mock_satellite/mock_satellite.pb.h"

namespace lightstep {
/**
 * Manages a server that acts like a satellite.
 */
class MockSatelliteHandle {
 public:
  explicit MockSatelliteHandle(uint16_t port);

  /**
   * @return the spans recorded by the satellite.
   */
  std::vector<collector::Span> spans();

  /**
   * @return the reports recorded by the satellite.
   */
  std::vector<collector::ReportRequest> reports();

 private:
  ChildProcessHandle handle_;
  HttpConnection connection_;
};
}  // namespace lightstep
