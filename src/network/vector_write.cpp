#include "network/vector_write.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

#include "common/platform/error.h"
#include "common/platform/memory.h"
#include "common/platform/network.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Write
//--------------------------------------------------------------------------------------------------
bool Write(int socket,
           std::initializer_list<FragmentInputStream*> fragment_input_streams) {
  int num_fragments = 0;
  for (auto fragment_input_stream : fragment_input_streams) {
    num_fragments += fragment_input_stream->num_fragments();
  }
  if (num_fragments == 0) {
    return true;
  }
  assert(num_fragments < IoVecMax);
  auto fragments = static_cast<IoVec*>(alloca(sizeof(IoVec) * num_fragments));
  auto fragment_iter = fragments;
  for (auto fragment_input_stream : fragment_input_streams) {
    fragment_input_stream->ForEachFragment(
        [&fragment_iter](void* data, int size) {
          *fragment_iter++ = MakeIoVec(data, static_cast<size_t>(size));
          return true;
        });
  }
  assert(fragment_iter == fragments + num_fragments);
  auto rcode = WriteV(socket, fragments, num_fragments);
  if (rcode < 0) {
    auto error_code = GetLastErrorCode();
    if (IsBlockingErrorCode(error_code)) {
      return false;
    }
    std::ostringstream oss;
    oss << "writev failed: " << GetErrorCodeMessage(error_code);
    throw std::runtime_error{oss.str()};
  }
  return Consume(fragment_input_streams, static_cast<int>(rcode));
}
}  // namespace lightstep
