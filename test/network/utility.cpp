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
std::string ToString(const FragmentInputStream& fragment_input_stream) {
  std::string result;
  fragment_input_stream.ForEachFragment([&result](void* data, int size) {
    result.append(static_cast<char*>(data), static_cast<size_t>(size));
    return true;
  });
  return result;
}
}  // namespace lightstep
