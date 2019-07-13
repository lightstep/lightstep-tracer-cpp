#include "network/socket.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>

//#include <fcntl.h>
#include <sys/types.h>
//#include <unistd.h>
#include <iostream>

#define _WINSOCKAPI_
#include "winsock2.h"

namespace lightstep {

void verifyWinsockVersion() {
  WSADATA wsaData;

  /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
  WORD wVersionRequested = MAKEWORD(2, 2);

  int err = WSAStartup(wVersionRequested, &wsaData);

	// if we couldn't find a winsock dll
  if (err != 0) {
    throw std::runtime_error{"we could not find a winsock dll"};
  }

  /* Confirm that the WinSock DLL supports 2.2.*/
  /* Note that if the DLL supports versions greater    */
  /* than 2.2 in addition to 2.2, it will still return */
  /* 2.2 in wVersion since that is the version we      */
  /* requested.                                        */
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
    WSACleanup();
		throw std::runtime_error{"Could not find a usable version of Winsock.dll"};
  }
	else {
    std::cout << "The Winsock 2.2 dll was found okay" << std::endl;
  }
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
Socket::Socket() : Socket{AF_INET, SOCK_STREAM} {}

Socket::Socket(int family, int type) {
  verifyWinsockVersion();

  SOCKET sock = INVALID_SOCKET;
  sock = socket(family, type, 0);

  if (sock == INVALID_SOCKET) {
    std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
    WSACleanup();
    return;
  } else {
    std::cout << "successfully created socket" << std::endl;
  }

  // NOW WE NEED TO BIND
}

Socket::Socket(int file_descriptor) noexcept
    : file_descriptor_{file_descriptor} {}

Socket::Socket(Socket&& other) noexcept {
  file_descriptor_ = other.file_descriptor_;
  other.file_descriptor_ = -1;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
Socket::~Socket() noexcept { Free(); }

//--------------------------------------------------------------------------------------------------
// operator=
//--------------------------------------------------------------------------------------------------
Socket& Socket::operator=(Socket&& other) noexcept {
  assert(this != &other);
  Free();
  file_descriptor_ = other.file_descriptor_;
  other.file_descriptor_ = -1;
  return *this;
}

//------------------------------------------------------------------------------
// SetNonblocking
//------------------------------------------------------------------------------
void Socket::SetNonblocking() {
  // set the nonblocking bit to 1 (flag)
  // https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-ioctlsocket
  unsigned long flag = 1;
  int rcode = ioctlsocket(file_descriptor_, FIONBIO, &flag);

  // make sure we successfully reconfigured the socket
  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as non-blocking: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }

  return;
}

//--------------------------------------------------------------------------------------------------
// Free
//--------------------------------------------------------------------------------------------------
void Socket::Free() noexcept {
  if (file_descriptor_ != -1) {
    closesocket(file_descriptor_);
  }
}

//------------------------------------------------------------------------------
// SetReuseAddress
//------------------------------------------------------------------------------
void Socket::SetReuseAddress() {
  // set SO_REUSEADDR to 1 to
  // TODO: find out what the level param does
  // https://docs.microsoft.com/en-us/windows/desktop/winsock/using-so-reuseaddr-and-so-exclusiveaddruse
  const char optvalue = 1;
  int rcode = setsockopt(file_descriptor_, SOL_SOCKET, SO_REUSEADDR, &optvalue,
                         sizeof(const char));

  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as reusable: : " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
}

//--------------------------------------------------------------------------------------------------
// Connect
//--------------------------------------------------------------------------------------------------
int Socket::Connect(const sockaddr& addr, size_t addrlen) noexcept {
  assert(file_descriptor_ != -1);
  return connect(file_descriptor_, &addr, (int)addrlen);
}

Socket Connect(const IpAddress& ip_address) {
  Socket socket{(int)ip_address.family(), SOCK_STREAM};
  socket.SetNonblocking();
  socket.SetReuseAddress();
  int rcode;
  switch (ip_address.family()) {
    case AF_INET:
      rcode =
          socket.Connect(ip_address.addr(), sizeof(ip_address.ipv4_address()));
      break;
    case AF_INET6:
      rcode =
          socket.Connect(ip_address.addr(), sizeof(ip_address.ipv6_address()));
      break;
    default:
      throw std::runtime_error{"Unknown socket family."};
  }
  if (rcode == 0) {
    return socket;
  }
  assert(rcode == -1);
  if (errno != EINPROGRESS) {
    std::ostringstream oss;
    oss << "connect failed: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
  return socket;
}
}  // namespace lightstep
