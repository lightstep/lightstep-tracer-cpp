#pragma once

#include <initializer_list>
#include <vector>

#include "common/fragment_input_stream.h"

namespace lightstep {
/**
 * A non-owning stream of fragments.
 */
class FragmentArrayInputStream final : public FragmentInputStream {
 public:
  FragmentArrayInputStream() noexcept = default;

  explicit FragmentArrayInputStream(std::initializer_list<Fragment> fragments);

  FragmentArrayInputStream& operator=(
      std::initializer_list<Fragment> fragments);

  /**
   * Reserve memory for fragments.
   * @param size the number of fragments to reserve memory for.
   */
  void Reserve(size_t size) { fragments_.reserve(size); }

  /**
   * Add a fragment to the stream.
   * @param fragment the fragment to add.
   */
  void Add(Fragment fragment) { fragments_.emplace_back(fragment); }

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(Callback callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override;

 private:
  std::vector<Fragment> fragments_;

  int fragment_index_{0};
  int position_{0};
};
}  // namespace lightstep
