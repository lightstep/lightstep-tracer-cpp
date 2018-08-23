#include "socket.h"

#include <sys/socket.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
Socket::Socket() {
  file_descriptor_ = socket(AF_INET, SOCK_STREAM, 0);
  if (file_descriptor_ < 0) {
    throw std::runtime_error{"failed to create socket"};
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
    throw std::runtime_error{"failed to set the socket as non-blocking"};
  }
}

//------------------------------------------------------------------------------
// SetBlocking
//------------------------------------------------------------------------------
void Socket::SetBlocking() {
  int rcode = fcntl(file_descriptor_, F_SETFL,
                    fcntl(file_descriptor_, F_GETFL, 0) & ~O_NONBLOCK);
  if (rcode == -1) {
    throw std::runtime_error{"failed to set the socket as blocking"};
  }
}
}  // namespace lightstep
