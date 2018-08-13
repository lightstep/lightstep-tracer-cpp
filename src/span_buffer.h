#pragma once

#include "circular_buffer.h"
#include "atomic_bit_set.h"

#include <google/protobuf/message.h>

namespace lightstep {
class SpanBuffer {
 public:
  using ConsumerFunction = size_t (*)(void* context, const char* data,
                                      size_t num_bytes);

  explicit SpanBuffer(size_t num_bytes);

  bool Add(const google::protobuf::Message& message) noexcept;

  bool Consume(ConsumerFunction consumer, void* context);

 private:
   CircularBuffer buffer_;
   AtomicBitSet ready_flags_;
   size_t consumer_allotment_ = 0;

   void GrowConsumerAllotment() noexcept;
};
} // namespace lightstep
