#include "common/protobuf.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// WriteEmbeddedMessage
//--------------------------------------------------------------------------------------------------
void WriteEmbeddedMessage(google::protobuf::io::CodedOutputStream& stream,
                          uint32_t field,
                          const google::protobuf::Message& message) {
  stream.WriteVarint32((field << 3) | 2);
  stream.WriteVarint64(message.ByteSizeLong());
  message.SerializeWithCachedSizes(&stream);
}
}  // namespace lightstep
