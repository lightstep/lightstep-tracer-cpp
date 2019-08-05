#include "tracer/allocator_manager.h"

#include <stdexcept>

#include "common/serialization_chain.h"
#include "tracer/span.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ComputeMaxSpans
//--------------------------------------------------------------------------------------------------
static size_t ComputeMaxSpans(size_t memory_limit) {
  if (memory_limit == 0) {
    return 0;
  }
  auto result = memory_limit / (sizeof(Span) + sizeof(SerializationChain));
  if (result == 0) {
    throw std::runtime_error{"memory limit is too low"};
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
AllocatorManager::AllocatorManager(size_t memory_limit)
    : span_allocator_{ComputeMaxSpans(memory_limit), sizeof(Span)},
      serialization_chain_allocator_{span_allocator_.max_blocks(),
                                     sizeof(SerializationChain)} {}
}  // namespace lightstep
