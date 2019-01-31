#pragma once

#include <cassert>
#include <iosfwd>

#include <arpa/inet.h>
#include <sys/socket.h>

namespace lightstep {
class IpAddress {
 public:
  IpAddress(const sockaddr& addr) noexcept;

  explicit IpAddress(const char* s);

  const sockaddr& addr() const noexcept {
    return reinterpret_cast<const sockaddr&>(data_);
  }

  sa_family_t family() const noexcept { return data_.ss_family; }

  const sockaddr_in& ipv4_address() const noexcept {
    assert(family() == AF_INET);
    return reinterpret_cast<const sockaddr_in&>(data_);
  }

  const sockaddr_in6& ipv6_address() const noexcept {
    assert(family() == AF_INET6);
    return reinterpret_cast<const sockaddr_in6&>(data_);
  }

  void set_port(uint16_t port) noexcept;

 private:
  sockaddr_storage data_;
};

bool operator==(const IpAddress& lhs, const IpAddress& rhs) noexcept;

inline bool operator!=(const IpAddress& lhs, const IpAddress& rhs) noexcept {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const IpAddress& ip_address);

std::string ToString(const IpAddress& ip_address);
}  // namespace lightstep
