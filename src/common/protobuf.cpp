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

//--------------------------------------------------------------------------------------------------
// ComputeEmbeddedMessageSerializationSize
//--------------------------------------------------------------------------------------------------
size_t ComputeEmbeddedMessageSerializationSize(
    uint32_t field, const google::protobuf::Message& message) {
  size_t result =
      google::protobuf::io::CodedOutputStream::VarintSize32((field << 3) | 2);
  auto message_size = message.ByteSizeLong();
  result +=
      google::protobuf::io::CodedOutputStream::VarintSize64(message_size) +
      message_size;
  return result;
}
}  // namespace lightstep
