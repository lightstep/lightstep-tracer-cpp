#include "test/network/utility.h"

#include <cstring>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeFragment
//--------------------------------------------------------------------------------------------------
std::pair<void*, int> MakeFragment(const char* s) {
  return {static_cast<void*>(const_cast<char*>(s)),
          static_cast<int>(std::strlen(s))};
}

//--------------------------------------------------------------------------------------------------
// ToString
//--------------------------------------------------------------------------------------------------
std::string ToString(const FragmentSet& fragment_set) {
  std::string result;
  fragment_set.ForEachFragment([&result](void* data, int size) {
    result.append(static_cast<char*>(data), static_cast<size_t>(size));
    return true;
  });
  return result;
}
}  // namespace lightstep
