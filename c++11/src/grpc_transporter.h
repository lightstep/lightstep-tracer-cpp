#pragma once

#include <collector.grpc.pb.h>
#include <collector.pb.h>
#include <grpc++/create_channel.h>
#include <lightstep/spdlog/logger.h>
#include <lightstep/tracer.h>
#include <lightstep/transporter.h>

namespace lightstep {
/**
 * GrpcTransporter sends ReportRequests to the specified host via gRPC.
 */
class GrpcTransporter : public Transporter {
 public:
  GrpcTransporter(spdlog::logger& logger,
                  const LightStepTracerOptions& options);

  opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) override;

 private:
  spdlog::logger& logger_;
  // Collector service stub.
  collector::CollectorService::Stub client_;
  std::chrono::system_clock::duration report_timeout_;
};
}  // namespace lightstep
