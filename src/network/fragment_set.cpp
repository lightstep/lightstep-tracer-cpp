#include "network/fragment_set.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ComputeFragmentPosition
//--------------------------------------------------------------------------------------------------
std::tuple<int, int, int> ComputeFragmentPosition(
    std::initializer_list<const FragmentSet*> fragment_sets, int n) noexcept {
  int fragment_set_index = 0;
  for (auto fragment_set : fragment_sets) {
    int fragment_index = 0;
    auto f = [&n, &fragment_index](void* /*data*/, int size) {
      if (n < size) {
        return false;
      }
      ++fragment_index;
      n -= size;
      return true;
    };
    if (!fragment_set->ForEachFragment(f)) {
      return std::make_tuple(fragment_set_index, fragment_index, n);
    }
    ++fragment_set_index;
  }
  return {static_cast<int>(fragment_sets.size()), 0, 0};
}
}  // namespace lightstep
