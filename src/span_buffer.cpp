#include "span_buffer.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SpanBuffer::SpanBuffer(size_t size) : buffer_{size}, ready_flags_{size} {}

//------------------------------------------------------------------------------
// Add
//------------------------------------------------------------------------------
bool SpanBuffer::Add(const google::protobuf::Message& message) noexcept {
  auto serialization_size = message.ByteSizeLong();
  (void)serialization_size;
  return true;
}
}  // namespace lightstep
