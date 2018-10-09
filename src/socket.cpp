#include "socket.h"

#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
Socket::Socket() {
  file_descriptor_ = socket(AF_INET, SOCK_STREAM, 0);
  if (file_descriptor_ < 0) {
    std::ostringstream oss;
    oss << "failed to create socket: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
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
Socket::~Socket() {
  if (file_descriptor_ != -1) {
    close(file_descriptor_);
  }
}

//------------------------------------------------------------------------------
// SetNonblocking
//------------------------------------------------------------------------------
void Socket::SetNonblocking() {
  int rcode = fcntl(file_descriptor_, F_SETFL,
                    fcntl(file_descriptor_, F_GETFL, 0) | O_NONBLOCK);
  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as non-blocking: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
}

//------------------------------------------------------------------------------
// SetBlocking
//------------------------------------------------------------------------------
void Socket::SetBlocking() {
  int rcode = fcntl(file_descriptor_, F_SETFL,
                    fcntl(file_descriptor_, F_GETFL, 0) & ~O_NONBLOCK);
  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as blocking: " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
}

//------------------------------------------------------------------------------
// SetReuseAddress
//------------------------------------------------------------------------------
void Socket::SetReuseAddress() {
  int optvalue = 1;
  int rcode = setsockopt(file_descriptor_, SOL_SOCKET, SO_REUSEADDR,
                         static_cast<void*>(&optvalue), sizeof(int));
  if (rcode == -1) {
    std::ostringstream oss;
    oss << "failed to set the socket as reusable: : " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
}
}  // namespace lightstep
