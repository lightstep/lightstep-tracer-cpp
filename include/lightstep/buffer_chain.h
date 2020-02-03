#pragma once

#include <cstddef>

namespace lightstep {
/**
 * BufferChain provides an interface to access a chained sequence of
 * contiguous memory buffers.
 */
class BufferChain {
 public:
  using FragmentCallback = bool (*)(void*, const void*, size_t);

  virtual ~BufferChain() = default;

  /**
   * @return the number of fragments in chain
   */
  virtual size_t num_fragments() const noexcept = 0;

  /**
   * @return the total number of bytes in all fragments
   */
  virtual size_t num_bytes() const noexcept = 0;

  /**
   * Iterate over each fragment in the buffer chain
   * @param callback the callback to call for each fragment
   * @param context a pointer to pass to the callback.
   */
  virtual bool ForEachFragment(FragmentCallback callback,
                               void* context) const = 0;

  /**
   * Copy out all the fragments into an area of contiguous memory
   * @param data the location to start copying
   * @param length the size of the buffer destination. Length must be at least
   * num_bytes.
   */
  void CopyOut(char* data, size_t length) const noexcept;
};
}  // namespace lightstep
