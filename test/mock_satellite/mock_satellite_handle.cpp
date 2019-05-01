#include "test/mock_satellite/mock_satellite_handle.h"

#include <exception>
#include <iostream>
#include <string>

#include "test/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
MockSatelliteHandle::MockSatelliteHandle(uint16_t port)
    : handle_{"test/mock_satellite/mock_satellite", {std::to_string(port)}},
      connection_{"127.0.0.1", port} {
  if (!IsEventuallyTrue([port] { return CanConnect(port); })) {
    std::cerr << "Failed to connect to mock satellite\n";
    std::terminate();
  }
}

//--------------------------------------------------------------------------------------------------
// spans
//--------------------------------------------------------------------------------------------------
std::vector<collector::Span> MockSatelliteHandle::spans() {
  mock_satellite::Spans spans;
  connection_.Get("/spans", spans);
  std::vector<collector::Span> result;
  result.reserve(spans.spans().size());
  for (auto& span : spans.spans()) {
    result.emplace_back(span);
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// reports
//--------------------------------------------------------------------------------------------------
std::vector<collector::ReportRequest> MockSatelliteHandle::reports() {
  mock_satellite::Reports reports;
  connection_.Get("/reports", reports);
  std::vector<collector::ReportRequest> result;
  result.reserve(reports.reports().size());
  for (auto& report : reports.reports()) {
    result.emplace_back(report);
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// SetRequestError
//--------------------------------------------------------------------------------------------------
void MockSatelliteHandle::SetRequestError() {
  connection_.Get("/error-on-next-report");
}

//--------------------------------------------------------------------------------------------------
// SetRequestTimeout
//--------------------------------------------------------------------------------------------------
void MockSatelliteHandle::SetRequestTimeout() {
  connection_.Get("/timeout-on-next-report");
}

//--------------------------------------------------------------------------------------------------
// SetRequestPrematureClose
//--------------------------------------------------------------------------------------------------
void MockSatelliteHandle::SetRequestPrematureClose() {
  connection_.Get("/premature-close-next-report");
}

//--------------------------------------------------------------------------------------------------
// SetThrottleReports
//--------------------------------------------------------------------------------------------------
void MockSatelliteHandle::SetThrottleReports() {
  connection_.Get("/throttle-reports");
}
}  // namespace lightstep
