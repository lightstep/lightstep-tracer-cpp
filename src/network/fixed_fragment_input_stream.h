#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <initializer_list>
#include <utility>

#include "network/fragment_input_stream.h"

namespace lightstep {
/**
 * A non-owning stream of fragments.
 */
template <size_t N>
class FixedFragmentInputStream final : public FragmentInputStream {
 public:
  FixedFragmentInputStream() noexcept {}

  explicit FixedFragmentInputStream(
      std::initializer_list<Fragment> fragments) noexcept {
    AssignFragments(fragments);
  }

  FixedFragmentInputStream& operator=(
      std::initializer_list<Fragment> fragments) noexcept {
    AssignFragments(fragments);
    return *this;
  }

  // FragmentInputStream
  int num_fragments() const noexcept override {
    return static_cast<int>(N) - fragment_index_;
  }

  bool ForEachFragment(FunctionRef<Callback> callback) const noexcept override {
    if (fragment_index_ >= static_cast<int>(N)) {
      return true;
    }

    auto current_fragment = fragments_[fragment_index_];
    auto result =
        callback(static_cast<void*>(static_cast<char*>(current_fragment.first) +
                                    position_),
                 current_fragment.second - position_);
    if (!result) {
      return false;
    }

    for (int i = fragment_index_ + 1; i < static_cast<int>(N); ++i) {
      if (!callback(fragments_[i].first, fragments_[i].second)) {
        return false;
      }
    }
    return true;
  }

  void Clear() noexcept override { fragment_index_ = static_cast<int>(N); }

  void Seek(int fragment_index, int position) noexcept override {
    assert(fragment_index < static_cast<int>(N));
    if (fragment_index_ == fragment_index) {
      position_ += position;
    } else {
      position_ = position;
    }
    fragment_index_ += fragment_index;
    assert(fragment_index_ < static_cast<int>(N));
    assert(position_ < fragments_[fragment_index].second);
  }

 private:
  std::array<Fragment, N> fragments_;
  int fragment_index_{static_cast<int>(N)};
  int position_{0};

  void AssignFragments(std::initializer_list<Fragment> fragments) noexcept {
    assert(fragments.size() <= N);
    fragment_index_ = static_cast<int>(N);
    for (auto iter = fragments.end(); iter != fragments.begin();) {
      fragments_[--fragment_index_] = *--iter;
    }
    position_ = 0;
  }
};
}  // namespace lightstep
