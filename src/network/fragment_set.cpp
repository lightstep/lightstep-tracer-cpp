#include "network/fragment_set.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// FragmentPositionHelper
//--------------------------------------------------------------------------------------------------
namespace {
struct FragmentPositionHelper {
  int* n;
  int fragment_index;
};
}  // namespace

//--------------------------------------------------------------------------------------------------
// ComputeFragmentPosition
//--------------------------------------------------------------------------------------------------
std::tuple<int, int, int> ComputeFragmentPosition(
    std::initializer_list<const FragmentSet*> fragment_sets, int n) noexcept {
  auto f = [](void* /*data*/, int size, void* context) {
    auto& position_helper = *static_cast<FragmentPositionHelper*>(context);
    if (*position_helper.n < size) {
      return false;
    }
    ++position_helper.fragment_index;
    *position_helper.n -= size;
    return true;
  };
  int fragment_set_index = 0;
  for (auto fragment_set : fragment_sets) {
    FragmentPositionHelper position_helper{&n, 0};
    if (!fragment_set->ForEachFragment(f, static_cast<void*>(&n))) {
      return std::make_tuple(fragment_set_index, position_helper.fragment_index,
                             *position_helper.n);
    }
  }
  return {static_cast<int>(fragment_sets.size()), 0, 0};
}
}  // namespace lightstep
