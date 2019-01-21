#include "recorder/grpc_transporter.h"

#include <stdexcept>

namespace lightstep {
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    Logger& /*logger*/, const LightStepTracerOptions& /*options*/) {
  throw std::runtime_error{
      "LightStep was not built with gRPC support, so a transporter must be "
      "supplied."};
}
}  // namespace lightstep
