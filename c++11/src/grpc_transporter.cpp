#include <grpc++/create_channel.h>
#include <lightstep/tracer.h>
#include <chrono>
#include <sstream>
#include "transporter.h"
using namespace opentracing;

namespace lightstep {
//------------------------------------------------------------------------------
// hostPortOf
//------------------------------------------------------------------------------
static std::string hostPortOf(const LightStepTracerOptions& options) {
  std::ostringstream os;
  os << options.collector_host << ":" << options.collector_port;
  return os.str();
}

//------------------------------------------------------------------------------
// GrpcTransporter
//------------------------------------------------------------------------------
namespace {
class GrpcTransporter : public Transporter {
 public:
  explicit GrpcTransporter(const LightStepTracerOptions& options)
      : client_(grpc::CreateChannel(
            hostPortOf(options),
            (options.collector_encryption == "tls")
                ? grpc::SslCredentials(grpc::SslCredentialsOptions())
                : grpc::InsecureChannelCredentials())),
        report_timeout_{options.report_timeout} {}

  expected<collector::ReportResponse> SendReport(
      const collector::ReportRequest& report) noexcept override {
    grpc::ClientContext context;
    collector::ReportResponse resp;
    context.set_fail_fast(true);
    context.set_deadline(std::chrono::system_clock::now() + report_timeout_);
    auto status = client_.Report(&context, report, &resp);
    if (!status.ok()) {
      std::cerr << "Report RPC failed: " << status.error_message();
      // TODO(rnburn): Is there a better error code for this?
      return make_unexpected(
          std::make_error_code(std::errc::network_unreachable));
    }
    return {std::move(resp)};
  }

 private:
  // Collector service stub.
  collector::CollectorService::Stub client_;
  std::chrono::system_clock::duration report_timeout_;
};
}  // namespace

std::unique_ptr<Transporter> make_grpc_transporter(
    const LightStepTracerOptions& options) {
  return std::unique_ptr<Transporter>(new GrpcTransporter(options));
}
}  // namespace lightstep
