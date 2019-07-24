#include "common/platform/string.h"

#include <string.h>

namespace lightstep {
int StrCaseCmp(const char* s1, const char* s2) noexcept {
  return ::_stricmp(s1, s2);
}
}  // namespace lightstep
