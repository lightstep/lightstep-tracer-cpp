#include "common/platform/fork.h"

#include <pthread.h>

namespace lightstep {
int AtFork(void (*prepare)(), void (*parent)(), void (*child)()) noexcept {
  return ::pthread_atfork(prepare, parent, child);
}
}  // namespace lightstep
