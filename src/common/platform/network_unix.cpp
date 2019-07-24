#include "common/platform/network.h"

#include <climits>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// IoVecMax
//--------------------------------------------------------------------------------------------------
const int IoVecMax = IOV_MAX;

//--------------------------------------------------------------------------------------------------
// MakeIoVec
//--------------------------------------------------------------------------------------------------
IoVec MakeIoVec(void* data, size_t length) noexcept {
  return IoVec{data, length};
}

//--------------------------------------------------------------------------------------------------
// Read
//--------------------------------------------------------------------------------------------------
int Read(FileDescriptor socket, void* data, size_t size) noexcept {
  return static_cast<int>(::read(socket, data, size));
}

//--------------------------------------------------------------------------------------------------
// WriteV
//--------------------------------------------------------------------------------------------------
int WriteV(FileDescriptor socket, const IoVec* iov, int iovcnt) noexcept {
  return ::writev(socket, iov, iovcnt);
}

//--------------------------------------------------------------------------------------------------
// SetSocketNonblocking
//--------------------------------------------------------------------------------------------------
int SetSocketNonblocking(FileDescriptor socket) noexcept {
  return ::fcntl(socket, F_SETFL, ::fcntl(socket, F_GETFL, 0) | O_NONBLOCK);
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
int CloseSocket(FileDescriptor socket) noexcept { return ::close(socket); }
}  // namespace lightstep
