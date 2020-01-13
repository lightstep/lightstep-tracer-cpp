#pragma once

#include <cstddef>

namespace lightstep {
class BufferChain {
 public:
  using FragmentCallback = bool (*)(void*, const char*, size_t);

  virtual ~BufferChain() = default;

  virtual size_t num_fragments() const noexcept = 0;

  virtual size_t num_bytes() const noexcept = 0;

  virtual bool ForEachFragment(FragmentCallback callback,
                               void* context) const = 0;

  void Copy(char* data, size_t length) const noexcept;
};
} // namespace lightstep
