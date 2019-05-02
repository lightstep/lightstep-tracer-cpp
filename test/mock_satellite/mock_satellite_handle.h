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

  /**
   * Forces the mock satellite to return an HTTP error code for the next report
   * request.
   */
  void SetRequestError();

  /**
   * Forces the mock satellite to timeout for the next report request.
   */
  void SetRequestTimeout();

  /**
   * Forces the mock satellite to prematurely close the next report request.
   */
  void SetRequestPrematureClose();

  /**
   * Instructs the satellite to read requests out more slowly
   */
  void SetThrottleReports();

 private:
  ChildProcessHandle handle_;
  HttpConnection connection_;
};
}  // namespace lightstep
