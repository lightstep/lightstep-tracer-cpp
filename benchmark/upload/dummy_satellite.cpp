#include "dummy_satellite.h"

#include <sstream>

#include "grpc_dummy_satellite.h"
#include "stream_dummy_satellite.h"

namespace lightstep {
//------------------------------------------------------------------------------
// MakeDummySatellite
//------------------------------------------------------------------------------
std::unique_ptr<DummySatellite> MakeDummySatellite(
    const configuration_proto::TracerConfiguration& configuration) {
  std::unique_ptr<DummySatellite> result;
  if (configuration.use_stream_recorder()) {
    result.reset(new StreamDummySatellite{
        configuration.collector_host().c_str(),
        static_cast<int>(configuration.collector_port())});
  } else {
    std::ostringstream oss;
    oss << configuration.collector_host() << ":"
        << configuration.collector_port();
    auto grpc_satellite = new GrpcDummySatellite{oss.str().c_str()};
    result.reset(grpc_satellite);
    grpc_satellite->Run();
  }
  return result;
}
}  // namespace lightstep
