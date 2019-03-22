#include "common/random_traverser.h"

#include <algorithm>
#include <numeric>

#include "common/random.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
RandomTraverser::RandomTraverser(int n) : indexes_(n) {
  std::iota(indexes_.begin(), indexes_.end(), 0);
}

//--------------------------------------------------------------------------------------------------
// ForEachIndex
//--------------------------------------------------------------------------------------------------
bool RandomTraverser::ForEachIndex(Callback callback) {
  if (indexes_.empty()) {
    return true;
  }
  auto n_minus_1 = static_cast<int>(indexes_.size()) - 1;
  for (int i = 0; i < n_minus_1; ++i) {
    std::uniform_int_distribution<int> distribution{i, n_minus_1};
    std::swap(indexes_[i], indexes_[distribution(GetRandomNumberGenerator())]);
    if (!callback(indexes_[i])) {
      return false;
    }
  }
  return callback(indexes_.back());
}
}  // namespace lightstep
