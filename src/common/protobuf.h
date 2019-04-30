#pragma once

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/message.h>

namespace lightstep {
/**
 * Serializes a protobuf message in the format that allows it to be embedded in
 * to another message.
 * @param stream the stream to serialize into.
 * @param field the field number for the message.
 * @param message the message to serialize.
 */
void WriteEmbeddedMessage(google::protobuf::io::CodedOutputStream& stream,
                          uint32_t field,
                          const google::protobuf::Message& message);

/**
 * Serializes a protobuf message in the format that allows it to be embedded in
 * to another message.
 * @param stream the stream to serialize into.
 * @param field the field number for the message.
 * @param size the serialization size of the message.
 * @param message the message to serialize.
 */
void WriteEmbeddedMessage(google::protobuf::io::CodedOutputStream& stream,
                          uint32_t field, size_t message_size,
                          const google::protobuf::Message& message);

/**
 * Computes the size of an embedded protobuf message serialization.
 * @param field the field number for the message.
 * @param message the embedded message.
 * @return the number of bytes it takes to serialize the message.
 */
size_t ComputeEmbeddedMessageSerializationSize(
    uint32_t field, const google::protobuf::Message& message);

/**
 * Computes the size of an embedded protobuf message serialization.
 * @param field the field number for the message.
 * @param size the serialization size of the message.
 * @return the number of bytes it takes to serialize the message.
 */
size_t ComputeEmbeddedMessageSerializationSize(uint32_t field,
                                               size_t message_size);
}  // namespace lightstep
