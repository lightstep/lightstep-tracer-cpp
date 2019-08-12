#pragma once

#include "network/ip_address.h"

namespace lightstep {
/**
 * Wraps a system socket structure.
 */
class Socket {
 public:
  Socket();

  Socket(int family, int type);

  explicit Socket(FileDescriptor file_descriptor) noexcept;

  Socket(const Socket&) = delete;

  Socket(Socket&& other) noexcept;

  ~Socket() noexcept;

  Socket& operator=(Socket&& other) noexcept;

  Socket& operator=(const Socket&) = delete;

  /**
   * @return the file descriptor for this socket.
   */
  FileDescriptor file_descriptor() const noexcept { return file_descriptor_; }

  /**
   * Makes the socket non-blocking.
   */
  void SetNonblocking();

  /**
   * Makes the socket's address be reusable.
   */
  void SetReuseAddress();

  /**
   * Establishes a connection to a given address.
   * @param addr supplies the sockaddr to connect to.
   * @param addrlen supplies the length of the address.
   * @return the return code of the system call ::connect.
   */
  int Connect(const sockaddr& addr, size_t addrlen) noexcept;

 private:
  FileDescriptor file_descriptor_{InvalidSocket};

  void Free() noexcept;
};

/**
 * Establishes a connetion to a provided address.
 * @param ip_address supplies the address to connect to.
 * @param type supplies the type of connection.
 * @return a non-blocking Socket for the connection.
 */
Socket Connect(const IpAddress& ip_address);
}  // namespace lightstep
