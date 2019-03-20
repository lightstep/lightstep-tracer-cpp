#pragma once

#include <vector>

#include "common/function_ref.h"

namespace lightstep {
class RandomSequencer {
 public:
  using Callback = FunctionRef<bool(int)>;

  explicit RandomSequencer(int n);

  bool ForEachIndex(Callback callback);

 private:
  std::vector<int> indexes_;
};
}  // namespace lightstep
