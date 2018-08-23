#pragma once

#include "atomic_bit_set.h"
#include "circular_buffer.h"

#include <google/protobuf/message.h>

namespace lightstep {
class MessageBuffer {
 public:
  using ConsumerFunction = size_t (*)(void* context, const char* data,
                                      size_t num_bytes);

  explicit MessageBuffer(size_t num_bytes);

  bool Add(const google::protobuf::Message& message) noexcept;

  bool Consume(ConsumerFunction consumer, void* context);

 private:
  CircularBuffer buffer_;
  AtomicBitSet ready_flags_;
  size_t consumer_allotment_ = 0;

  void GrowConsumerAllotment() noexcept;
};
}  // namespace lightstep
