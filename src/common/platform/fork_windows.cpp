#include "common/platform/fork.h"

namespace lightstep {
int AtFork(void (*prepare)(), void (*parent)(), void (*child)()) noexcept {
  (void)prepare;
  (void)parent;
  (void)child;
  return 0;
}
}  // namespace lightstep
