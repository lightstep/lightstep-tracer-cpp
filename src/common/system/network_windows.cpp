#include "common/system/network.h"

#include <sstream>
#include <stdexcept>

#include <winsock2.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Read
//--------------------------------------------------------------------------------------------------
int Read(FileDescriptor socket, void* data, size_t size) noexcept {
  return ::recv(socket, static_cast<char*>(data), static_cast<int>(size), 0);
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
} // namespace lightstep
