#include "network/vector_write.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include <alloca.h>
#include <sys/uio.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Write
//--------------------------------------------------------------------------------------------------
std::tuple<int, int, int> Write(
    int socket, std::initializer_list<const FragmentSet*> fragment_sets) {
  int num_fragments = 0;
  for (auto fragment_set : fragment_sets) {
    num_fragments += fragment_set->num_fragments();
  }
  if (num_fragments == 0) {
    return std::make_tuple(0, 0, 0);
  }
  auto fragments = static_cast<iovec*>(alloca(sizeof(iovec) * num_fragments));
  auto fragment_iter = fragments;
  for (auto fragment_set : fragment_sets) {
    fragment_set->ForEachFragment([&fragment_iter](void* data, int size) {
      *fragment_iter++ = iovec{data, static_cast<size_t>(size)};
      return true;
    });
  }
  assert(fragment_iter == fragments + num_fragments);
  auto rcode = ::writev(socket, fragments, num_fragments);
  if (rcode == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return std::make_tuple(0, 0, 0);
    }
    std::ostringstream oss;
    oss << "writev failed: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
  return ComputeFragmentPosition(fragment_sets, static_cast<int>(rcode));
}
}  // namespace lightstep
