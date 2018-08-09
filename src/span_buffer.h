#pragma once

#include "circular_buffer.h"
#include "atomic_bit_set.h"

#include <google/protobuf/message.h>

namespace lightstep {
class SpanBuffer {
 public:
   explicit SpanBuffer(size_t num_bytes);


   bool Add(const google::protobuf::Message& message) noexcept;

 private:
   CircularBuffer buffer_;
   AtomicBitSet ready_flags_;
};
} // namespace lightstep
