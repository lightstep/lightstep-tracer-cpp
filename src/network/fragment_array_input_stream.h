#pragma once

#include <initializer_list>
#include <vector>

#include "network/fragment_input_stream.h"

namespace lightstep {
/**
 * A non-owning stream of fragments.
 */
class FragmentArrayInputStream final : public FragmentInputStream {
 public:
  FragmentArrayInputStream() noexcept = default;

  explicit FragmentArrayInputStream(
      std::initializer_list<Fragment> fragments) noexcept;

  FragmentArrayInputStream& operator=(
      std::initializer_list<Fragment> fragments) noexcept;

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(FunctionRef<Callback> callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override;

 private:
  std::vector<Fragment> fragments_;

  int fragment_index_{0};
  int position_{0};
};
}  // namespace lightstep
