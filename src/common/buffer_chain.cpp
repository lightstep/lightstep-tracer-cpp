#include "lightstep/buffer_chain.h"

#include <algorithm>
#include <exception>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// CopyOut
//--------------------------------------------------------------------------------------------------
void BufferChain::CopyOut(char* data, size_t length) const noexcept {
  if (length < this->num_bytes()) {
    std::terminate();
  }
  auto callback = [](void* context, const void* fragment_data,
                     size_t fragment_size) noexcept {
    auto out = static_cast<char**>(context);
    *out = std::copy_n(static_cast<const char*>(fragment_data), fragment_size,
                       *out);
    return true;
  };
  this->ForEachFragment(callback, static_cast<void*>(&data));
}
}  // namespace lightstep
