#include "test/network/utility.h"

#include <cstring>

namespace lightstep {
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
