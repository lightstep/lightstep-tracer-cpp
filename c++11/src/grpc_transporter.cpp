#include "grpc_transporter.h"
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
// Constructor
//------------------------------------------------------------------------------
GrpcTransporter::GrpcTransporter(spdlog::logger& logger,
                                 const LightStepTracerOptions& options)
    : logger_{logger},
      client_{grpc::CreateChannel(
          HostPortOf(options),
          (options.collector_encryption == "tls")
              ? grpc::SslCredentials(grpc::SslCredentialsOptions())
              : grpc::InsecureChannelCredentials())},
      report_timeout_{options.report_timeout} {}

GrpcTransporter2::GrpcTransporter2(spdlog::logger& logger,
                                   const LightStepTracerOptions& options)
    : logger_{logger},
      client_{grpc::CreateChannel(
          HostPortOf(options),
          (options.collector_encryption == "tls")
              ? grpc::SslCredentials(grpc::SslCredentialsOptions())
              : grpc::InsecureChannelCredentials())},
      report_timeout_{options.report_timeout} {}

//------------------------------------------------------------------------------
// SendReport
//------------------------------------------------------------------------------
opentracing::expected<collector::ReportResponse> GrpcTransporter::SendReport(
    const collector::ReportRequest& report) {
  grpc::ClientContext context;
  collector::ReportResponse resp;
  context.set_fail_fast(true);
  context.set_deadline(std::chrono::system_clock::now() + report_timeout_);
  auto status = client_.Report(&context, report, &resp);
  if (!status.ok()) {
    logger_.error("Report RPC failed: {}", status.error_message());
    // TODO(rnburn): Is there a better error code for this?
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::network_unreachable));
  }
  return {std::move(resp)};
}

//------------------------------------------------------------------------------
// Send
//------------------------------------------------------------------------------
opentracing::expected<void> GrpcTransporter2::Send(
    const google::protobuf::Message& request,
    google::protobuf::Message& response) {
  grpc::ClientContext context;
  collector::ReportResponse resp;
  context.set_fail_fast(true);
  context.set_deadline(std::chrono::system_clock::now() + report_timeout_);
  auto status = client_.Report(
      &context, static_cast<const collector::ReportRequest&>(request),
      static_cast<collector::ReportResponse*>(&response));
  if (!status.ok()) {
    logger_.error("Report RPC failed: {}", status.error_message());
    // TODO(rnburn): Is there a better error code for this?
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::network_unreachable));
  }
  return {};
}
}  // namespace lightstep
