#include "network/vector_write.h"

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstring>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <alloca.h>
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
  const auto max_batch_size =
      std::min(static_cast<int>(IOV_MAX), num_fragments);
  auto fragments = static_cast<iovec*>(alloca(sizeof(iovec) * max_batch_size));
  auto fragment_iter = fragments;
  const auto fragment_last = fragments + max_batch_size;
  int num_bytes_written = 0;
  int batch_num_bytes = 0;
  bool error = false;
  bool blocked = false;

  auto do_write = [&]() noexcept {
    auto rcode =
        ::writev(socket, fragments,
                 static_cast<int>(std::distance(fragments, fragment_iter)));
    if (rcode == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        blocked = true;
      } else {
        error = true;
      }
      return;
    }
    auto num_batch_bytes_written = static_cast<int>(rcode);
    assert(num_batch_bytes_written <= batch_num_bytes);
    num_bytes_written += num_batch_bytes_written;
    if (num_batch_bytes_written < batch_num_bytes) {
      blocked = true;
    }
  };

  for (auto fragment_input_stream : fragment_input_streams) {
    fragment_input_stream->ForEachFragment([&](void* data, int size) {
      *fragment_iter++ = iovec{data, static_cast<size_t>(size)};
      batch_num_bytes += size;
      if (fragment_iter != fragment_last) {
        return true;
      }
      do_write();
      fragment_iter = fragments;
      batch_num_bytes = 0;
      return !(error || blocked);
    });
  }
  if (batch_num_bytes > 0) {
    do_write();
  }
  auto result = Consume(fragment_input_streams, num_bytes_written);
  if (error) {
    std::ostringstream oss;
    oss << "writev failed: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
  return result;
}
}  // namespace lightstep
