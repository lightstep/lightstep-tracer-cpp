#include "common/platform/string.h"

#include <strings.h>

namespace lightstep {
int StrCaseCmp(const char* s1, const char* s2) noexcept {
  return ::strcasecmp(s1, s2);
}
}  // namespace lightstep
