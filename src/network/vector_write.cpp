#include "network/vector_write.h"

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "common/system/memory.h"

#include <sys/uio.h>

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
  assert(num_fragments < IOV_MAX);
  auto fragments = static_cast<iovec*>(alloca(sizeof(iovec) * num_fragments));
  auto fragment_iter = fragments;
  for (auto fragment_input_stream : fragment_input_streams) {
    fragment_input_stream->ForEachFragment(
        [&fragment_iter](void* data, int size) {
          *fragment_iter++ = iovec{data, static_cast<size_t>(size)};
          return true;
        });
  }
  assert(fragment_iter == fragments + num_fragments);
  auto rcode = ::writev(socket, fragments, num_fragments);
  if (rcode == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return false;
    }
    std::ostringstream oss;
    oss << "writev failed: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
  return Consume(fragment_input_streams, static_cast<int>(rcode));
}
}  // namespace lightstep
