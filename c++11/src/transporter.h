#ifndef LIGHTSTEP_TRANSPORT_H
#define LIGHTSTEP_TRANSPORT_H

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

std::unique_ptr<Transporter> make_grpc_transporter(
    const LightStepTracerOptions& options);
}  // namespace lightstep

#endif  // LIGHTSTEP_TRANSPORT_H
