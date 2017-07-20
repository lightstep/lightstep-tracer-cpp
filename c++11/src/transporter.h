#pragma once

#include <collector.grpc.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/util.h>

namespace lightstep {
class Transporter {
 public:
  virtual ~Transporter() = default;

  virtual opentracing::expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) noexcept = 0;
};
}  // namespace lightstep
