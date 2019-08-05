#pragma once

#include "common/block_allocator.h"

namespace lightstep {
class AllocatorManager {
 public:
   explicit AllocatorManager(size_t memory_limit);

   BlockAllocator& span_allocator() noexcept { return span_allocator_; }

   BlockAllocator& serialization_chain_allocator() noexcept {
     return serialization_chain_allocator_;
   }

  private:
   BlockAllocator span_allocator_;
   BlockAllocator serialization_chain_allocator_;
};
} // namespace lightstep
