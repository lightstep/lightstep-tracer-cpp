#include <lightstep-tracer-common/collector.pb.h>
#include <lightstep/transporter.h>

namespace lightstep {
//------------------------------------------------------------------------------
// MakeCollectorResponse
//------------------------------------------------------------------------------
std::unique_ptr<google::protobuf::Message>
Transporter::MakeCollectorResponse() {
  return std::unique_ptr<google::protobuf::Message>{
      new collector::ReportResponse{}};
}
}  // namespace lightstep
