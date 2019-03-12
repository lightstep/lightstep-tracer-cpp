#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/message.h>

namespace lightstep {
void WriteEmbeddedMessage(google::protobuf::io::CodedOutputStream& stream,
                          uint32_t field,
                          const google::protobuf::Message& message);

size_t ComputeEmbeddedMessageSerializationSize(
    uint32_t field, const google::protobuf::Message& message);
}  // namespace lightstep
