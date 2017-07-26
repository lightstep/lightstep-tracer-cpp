#pragma once

#include <collector.grpc.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/util.h>

namespace lightstep {
/**
 * An Abstract class that sends ReportRequests.
 */
class Transporter {
 public:
  virtual ~Transporter() = default;

  virtual opentracing::expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) = 0;
};
}  // namespace lightstep
