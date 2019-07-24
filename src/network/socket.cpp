#include "network/socket.h"

#include "common/platform/error.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
Socket::Socket() : Socket{AF_INET, SOCK_STREAM} {}

Socket::Socket(int family, int type) {
  file_descriptor_ = ::socket(family, type, 0);
  if (file_descriptor_ == InvalidSocket) {
    std::ostringstream oss;
    oss << "failed to create socket: "
        << GetErrorCodeMessage(GetLastErrorCode());
    throw std::runtime_error{oss.str()};
  }
}

Socket::Socket(FileDescriptor file_descriptor) noexcept
    : file_descriptor_{file_descriptor} {}

Socket::Socket(Socket&& other) noexcept {
  file_descriptor_ = other.file_descriptor_;
  other.file_descriptor_ = InvalidSocket;
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
  other.file_descriptor_ = InvalidSocket;
  return *this;
}

//------------------------------------------------------------------------------
// SetNonblocking
//------------------------------------------------------------------------------
void Socket::SetNonblocking() {
  auto rcode = SetSocketNonblocking(file_descriptor_);
  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as non-blocking: "
        << GetErrorCodeMessage(GetLastErrorCode());
    throw std::runtime_error{oss.str()};
  }
}

//--------------------------------------------------------------------------------------------------
// Free
//--------------------------------------------------------------------------------------------------
void Socket::Free() noexcept {
  if (file_descriptor_ != InvalidSocket) {
    CloseSocket(file_descriptor_);
  }
}

//------------------------------------------------------------------------------
// SetReuseAddress
//------------------------------------------------------------------------------
void Socket::SetReuseAddress() {
  auto rcode = SetSocketReuseAddress(file_descriptor_);
  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as reusable: : "
        << GetErrorCodeMessage(GetLastErrorCode());
    throw std::runtime_error{oss.str()};
  }
}

//--------------------------------------------------------------------------------------------------
// Connect
//--------------------------------------------------------------------------------------------------
int Socket::Connect(const sockaddr& addr, size_t addrlen) noexcept {
  assert(file_descriptor_ != InvalidSocket);
  return ::connect(file_descriptor_, &addr, static_cast<socklen_t>(addrlen));
}

Socket Connect(const IpAddress& ip_address) {
  Socket socket{ip_address.family(), SOCK_STREAM};
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
  auto error_code = GetLastErrorCode();
  if (!IsInProgressErrorCode(error_code) && !IsBlockingErrorCode(error_code)) {
    std::ostringstream oss;
    oss << "connect failed: " << GetErrorCodeMessage(error_code);
    throw std::runtime_error{oss.str()};
  }
  return socket;
}
}  // namespace lightstep
