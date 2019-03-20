#include "common/protobuf.h"

namespace lightstep {
static const uint32_t WireTypeLengthDelimited = 2;
static const int ProtoFieldShift = 3;

//--------------------------------------------------------------------------------------------------
// ComputeStreamMessageKey
//--------------------------------------------------------------------------------------------------
static uint32_t ComputeStreamMessageKey(uint32_t field, uint32_t wire_type) {
  // See description of encoding process at
  // https://developers.google.com/protocol-buffers/docs/encoding
  return (field << ProtoFieldShift) | wire_type;
}

//--------------------------------------------------------------------------------------------------
// WriteEmbeddedMessage
//--------------------------------------------------------------------------------------------------
void WriteEmbeddedMessage(google::protobuf::io::CodedOutputStream& stream,
                          uint32_t field,
                          const google::protobuf::Message& message) {
  WriteEmbeddedMessage(stream, field, message.ByteSizeLong(), message);
}

void WriteEmbeddedMessage(google::protobuf::io::CodedOutputStream& stream,
                          uint32_t field, size_t message_size,
                          const google::protobuf::Message& message) {
  stream.WriteVarint32(ComputeStreamMessageKey(field, WireTypeLengthDelimited));
  stream.WriteVarint64(message_size);
  message.SerializeWithCachedSizes(&stream);
}

//--------------------------------------------------------------------------------------------------
// ComputeEmbeddedMessageSerializationSize
//--------------------------------------------------------------------------------------------------
size_t ComputeEmbeddedMessageSerializationSize(
    uint32_t field, const google::protobuf::Message& message) {
  return ComputeEmbeddedMessageSerializationSize(field, message.ByteSizeLong());
}

size_t ComputeEmbeddedMessageSerializationSize(uint32_t field,
                                               size_t message_size) {
  size_t result = google::protobuf::io::CodedOutputStream::VarintSize32(
      ComputeStreamMessageKey(field, WireTypeLengthDelimited));
  result +=
      google::protobuf::io::CodedOutputStream::VarintSize64(message_size) +
      message_size;
  return result;
}
}  // namespace lightstep
