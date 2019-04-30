#pragma once

#include <vector>

#include "common/function_ref.h"

namespace lightstep {
/**
 * Iterate over indexes in random order.
 */
class RandomTraverser {
 public:
  using Callback = FunctionRef<bool(int)>;

  explicit RandomTraverser(int n);

  /**
   * Iterates over over indexes in random order.
   * @param callback the callback to call with each index. Callback can return
   * false to exit early.
   * @return false if callback exited early; otherwise, true.
   */
  bool ForEachIndex(Callback callback);

 private:
  std::vector<int> indexes_;
};
}  // namespace lightstep
