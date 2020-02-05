#include "common/report_request_framing.h"

#include <cassert>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// WriteReportRequestSpansHeader
//--------------------------------------------------------------------------------------------------
size_t WriteReportRequestSpansHeader(char* data, size_t size,
                                     uint32_t body_size) noexcept {
  assert(size >= ReportRequestSpansMaxHeaderSize);
  auto header_size =
      ComputeLengthDelimitedHeaderSerializationSize<ReportRequestSpansField>(
          body_size);
  auto serialization_start = data + (size - header_size);
  DirectCodedOutputStream stream{
      reinterpret_cast<google::protobuf::uint8*>(serialization_start)};
  WriteKeyLength<ReportRequestSpansField>(stream, body_size);
  return header_size;
}
}  // namespace lightstep
