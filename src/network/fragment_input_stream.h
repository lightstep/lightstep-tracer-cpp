#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <initializer_list>
#include <utility>

#include "network/fragment_set.h"

namespace lightstep {
template <size_t N>
class FragmentInputStream : public FragmentSet {
 public:
  explicit FragmentInputStream(
      std::initializer_list<std::pair<void*, int>> fragments) {
    assert(fragments.size() == N);
    std::copy(fragments.begin(), fragments.end(), fragments_.begin());
  }

  void Seek(int fragment_index, int position) noexcept {
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

  void Clear() { fragment_index_ = static_cast<int>(N); }

  // FragmentSet
  int num_fragments() const noexcept override {
    return static_cast<int>(N) - fragment_index_;
  }

  bool ForEachFragment(Callback callback, void* context) const
      noexcept override {
    if (fragment_index_ >= static_cast<int>(N)) {
      return true;
    }

    auto current_fragment = fragments_[fragment_index_];
    auto result =
        callback(static_cast<void*>(static_cast<char*>(current_fragment.first) +
                                    position_),
                 current_fragment.second - position_, context);
    if (!result) {
      return false;
    }

    for (int i = fragment_index_ + 1; i < static_cast<int>(N); ++i) {
      if (!callback(fragments_[i].first, fragments_[i].second, context)) {
        return false;
      }
    }
    return true;
  }

 private:
  std::array<std::pair<void*, int>, N> fragments_;
  int fragment_index_{0};
  int position_{0};
};
}  // namespace lightstep
