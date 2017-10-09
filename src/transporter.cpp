#include <collector.pb.h>
#include <lightstep/transporter.h>

namespace lightstep {
//------------------------------------------------------------------------------
// MakeCollectorResponse
//------------------------------------------------------------------------------
std::unique_ptr<google::protobuf::Message> Transporter::MakeCollectorResponse(
    const google::protobuf::Message& /*request*/) {
  return std::unique_ptr<google::protobuf::Message>{
      new collector::ReportResponse{}};
}
}  // namespace lightstep
