#pragma once

#include "network/ip_address.h"

namespace lightstep {
class Socket {
 public:
  Socket();

  Socket(int family, int type);

  explicit Socket(int file_descriptor) noexcept;

  Socket(const Socket&) = delete;

  Socket(Socket&& other) noexcept;

  ~Socket() noexcept;

  Socket& operator=(Socket&& other) noexcept = delete;

  Socket& operator=(const Socket&) = delete;

  int file_descriptor() const noexcept { return file_descriptor_; }

  void SetNonblocking();

  void SetBlocking();

  void SetReuseAddress();

  void Connect(const sockaddr& addr, size_t addrlen);

 private:
  int file_descriptor_{-1};
};

Socket Connect(const IpAddress& ip_address, int type);
}  // namespace lightstep
