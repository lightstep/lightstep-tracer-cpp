#include "grpc_transporter.h"
#include <lightstep/config.h>

#ifdef LS_WITH_GRPC
#include <collector.grpc.pb.h>
#include <collector.pb.h>
#include <grpc++/create_channel.h>
#include <chrono>
#include <sstream>

namespace lightstep {
//------------------------------------------------------------------------------
// HostPortOf
//------------------------------------------------------------------------------
static std::string HostPortOf(const LightStepTracerOptions& options) {
  std::ostringstream os;
  os << options.collector_host << ":" << options.collector_port;
  return os.str();
}

//------------------------------------------------------------------------------
// GrpcTransporter
//------------------------------------------------------------------------------
namespace {
/**
 * GrpcTransporter sends ReportRequests to the specified host via gRPC.
 */
class GrpcTransporter : public SyncTransporter {
 public:
  GrpcTransporter(spdlog::logger& logger, const LightStepTracerOptions& options)
      : logger_{logger},
        client_{grpc::CreateChannel(
            HostPortOf(options),
            (options.collector_encryption == "tls")
                ? grpc::SslCredentials(grpc::SslCredentialsOptions())
                : grpc::InsecureChannelCredentials())},
        report_timeout_{options.report_timeout} {}

  opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) override {
    grpc::ClientContext context;
    collector::ReportResponse resp;
    context.set_fail_fast(true);
    context.set_deadline(std::chrono::system_clock::now() + report_timeout_);
    auto status = client_.Report(
        &context, dynamic_cast<const collector::ReportRequest&>(request),
        dynamic_cast<collector::ReportResponse*>(&response));
    if (!status.ok()) {
      logger_.error("Report RPC failed: {}", status.error_message());
      // TODO(rnburn): Is there a better error code for this?
      return opentracing::make_unexpected(
          std::make_error_code(std::errc::network_unreachable));
    }
    return {};
  }

 private:
  spdlog::logger& logger_;
  // Collector service stub.
  collector::CollectorService::Stub client_;
  std::chrono::system_clock::duration report_timeout_;
};
}  // anonymous namespace

//------------------------------------------------------------------------------
// MakeGrpcTransporter
//------------------------------------------------------------------------------
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    spdlog::logger& logger, const LightStepTracerOptions& options) {
  return std::unique_ptr<SyncTransporter>{new GrpcTransporter{logger, options}};
}
}  // namespace lightstep
#else
#include <stdexcept>
namespace lightstep {
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    spdlog::logger& logger, const LightStepTracerOptions& options) {
  throw std::runtime_error{
      "LightStep was not built with gRPC support, so a transporter must be "
      "supplied."};
}
}  // namespace lightstep
#endif
