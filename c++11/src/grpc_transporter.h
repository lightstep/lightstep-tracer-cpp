#pragma once

#include <grpc++/create_channel.h>
#include <lightstep/tracer.h>
#include "transporter.h"

namespace lightstep {
/**
 * GrpcTransporter sends ReportRequests to the specified host via gRPC.
 */
class GrpcTransporter : public Transporter {
 public:
  explicit GrpcTransporter(const LightStepTracerOptions& options);

  opentracing::expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) override;

 private:
  // Collector service stub.
  collector::CollectorService::Stub client_;
  std::chrono::system_clock::duration report_timeout_;
};
}  // namespace lightstep
