#include "common/system/network.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Read
//--------------------------------------------------------------------------------------------------
int Read(FileDescriptor socket, void* data, size_t size) noexcept {
  return static_cast<int>(::read(socket, data, size));
}

//--------------------------------------------------------------------------------------------------
// SetSocketNonblocking
//--------------------------------------------------------------------------------------------------
int SetSocketNonblocking(FileDescriptor socket) noexcept {
  return ::fcntl(socket, F_SETFL,
                      ::fcntl(socket, F_GETFL, 0) | O_NONBLOCK);
}

//--------------------------------------------------------------------------------------------------
// SetSocketReuseAddress
//--------------------------------------------------------------------------------------------------
int SetSocketReuseAddress(FileDescriptor socket) noexcept {
  int optvalue = 1;
  return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
                      static_cast<void*>(&optvalue), sizeof(int));
}

//--------------------------------------------------------------------------------------------------
// CloseSocket
//--------------------------------------------------------------------------------------------------
int CloseSocket(FileDescriptor socket) noexcept {
  return ::close(socket);
}
} // namespace lightstep
