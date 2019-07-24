#include "common/platform/network.h"

#include <sstream>
#include <stdexcept>

#include <winsock2.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeIoVec
//--------------------------------------------------------------------------------------------------
IoVec MakeIoVec(void* data, size_t length) noexcept {
  return IoVec{static_cast<ULONG>(length), static_cast<char*>(data)};
}

//--------------------------------------------------------------------------------------------------
// Read
//--------------------------------------------------------------------------------------------------
int Read(FileDescriptor socket, void* data, size_t size) noexcept {
  return ::recv(socket, static_cast<char*>(data), static_cast<int>(size), 0);
}

//--------------------------------------------------------------------------------------------------
// WriteV
//--------------------------------------------------------------------------------------------------
int WriteV(FileDescriptor socket, const IoVec* iov, int iovcnt) noexcept {
  DWORD num_bytes_sent;
  auto rcode = ::WSASend(socket, const_cast<IoVec*>(iov), iovcnt,
                         &num_bytes_sent, 0, nullptr, nullptr);
  if (rcode != 0) {
    return rcode;
  }
  return static_cast<int>(num_bytes_sent);
}

//--------------------------------------------------------------------------------------------------
// SetSocketNonblocking
//--------------------------------------------------------------------------------------------------
int SetSocketNonblocking(FileDescriptor socket) noexcept {
  u_long nonblocking_mode = 1;
  return ::ioctlsocket(socket, FIONBIO, &nonblocking_mode);
}

//--------------------------------------------------------------------------------------------------
// SetSocketReuseAddress
//--------------------------------------------------------------------------------------------------
int SetSocketReuseAddress(FileDescriptor socket) noexcept {
  BOOL optvalue = TRUE;
  return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
                      reinterpret_cast<char*>(&optvalue), sizeof(optvalue));
}

//--------------------------------------------------------------------------------------------------
// CloseSocket
//--------------------------------------------------------------------------------------------------
int CloseSocket(FileDescriptor socket) noexcept {
  return ::closesocket(socket);
}
}  // namespace lightstep
