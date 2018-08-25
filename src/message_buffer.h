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

  size_t num_pending_bytes() const noexcept { return buffer_.size(); }

  size_t total_bytes_consumed() const noexcept { return total_bytes_consumed_; }

  bool empty() const noexcept { return buffer_.empty(); }

 private:
  CircularBuffer buffer_;
  AtomicBitSet ready_flags_;
  size_t consumer_allotment_ = 0;
  std::atomic<size_t> total_bytes_consumed_{0};

  void GrowConsumerAllotment() noexcept;
};
}  // namespace lightstep
