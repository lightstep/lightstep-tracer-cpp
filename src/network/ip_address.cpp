#include "network/ip_address.h"

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
IpAddress::IpAddress() noexcept {
  auto& ipv4_addr = reinterpret_cast<sockaddr_in&>(data_);
  ipv4_addr.sin_family = AF_INET;
}

IpAddress::IpAddress(const in_addr& addr) noexcept {
  auto& ipv4_addr = reinterpret_cast<sockaddr_in&>(data_);
  ipv4_addr.sin_family = AF_INET;
  ipv4_addr.sin_addr = addr;
}

IpAddress::IpAddress(const in6_addr& addr) noexcept {
  auto& ipv6_addr = reinterpret_cast<sockaddr_in6&>(data_);
  ipv6_addr.sin6_family = AF_INET6;
  ipv6_addr.sin6_addr = addr;
}

IpAddress::IpAddress(const sockaddr& addr) noexcept {
  if (addr.sa_family == AF_INET) {
    reinterpret_cast<sockaddr_in&>(data_) =
        reinterpret_cast<const sockaddr_in&>(addr);
  } else if (addr.sa_family == AF_INET6) {
    reinterpret_cast<sockaddr_in6&>(data_) =
        reinterpret_cast<const sockaddr_in6&>(addr);
  } else {
    assert(0 && "Unknown address family");
  }
}

IpAddress::IpAddress(const char* s) {
  data_ = {};
  auto& ipv4_addr = reinterpret_cast<sockaddr_in&>(data_);
  auto rcode = inet_pton(AF_INET, s, static_cast<void*>(&ipv4_addr.sin_addr));
  if (rcode == 1) {
    ipv4_addr.sin_family = AF_INET;
    return;
  }

  auto& ipv6_addr = reinterpret_cast<sockaddr_in6&>(data_);
  rcode = inet_pton(AF_INET6, s, static_cast<void*>(&ipv6_addr.sin6_addr));
  if (rcode == 1) {
    ipv6_addr.sin6_family = AF_INET6;
    return;
  }

  std::ostringstream oss;
  oss << "failed to construct IpAddress from " << s;
  throw std::runtime_error{oss.str()};
}

IpAddress::IpAddress(const char* s, uint16_t port) : IpAddress{s} {
  set_port(port);
}

//--------------------------------------------------------------------------------------------------
// set_port
//--------------------------------------------------------------------------------------------------
void IpAddress::set_port(uint16_t port) noexcept {
  if (family() == AF_INET) {
    reinterpret_cast<sockaddr_in&>(data_).sin_port = htons(port);
    return;
  }
  if (family() == AF_INET6) {
    reinterpret_cast<sockaddr_in6&>(data_).sin6_port = htons(port);
    return;
  }
  assert(0 && "Unknown address family");
}

//--------------------------------------------------------------------------------------------------
// operator==
//--------------------------------------------------------------------------------------------------
bool operator==(const IpAddress& lhs, const IpAddress& rhs) noexcept {
  if (lhs.family() != rhs.family()) {
    return false;
  }
  if (lhs.family() == AF_INET) {
    return std::memcmp(static_cast<const void*>(&lhs.ipv4_address()),
                       static_cast<const void*>(&rhs.ipv4_address()),
                       sizeof(lhs.ipv4_address())) == 0;
  }
  if (lhs.family() == AF_INET6) {
    return std::memcmp(static_cast<const void*>(&lhs.ipv6_address()),
                       static_cast<const void*>(&rhs.ipv6_address()),
                       sizeof(lhs.ipv6_address())) == 0;
  }
  assert(0 && "Unknown address family");
  return false;
}

//--------------------------------------------------------------------------------------------------
// operator<<
//--------------------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const IpAddress& ip_address) {
  std::array<char, INET6_ADDRSTRLEN> buffer = {};
  const char* s = nullptr;
  if (ip_address.family() == AF_INET) {
    s = inet_ntop(AF_INET, &ip_address.ipv4_address().sin_addr, buffer.data(),
                  sizeof(buffer));
  } else if (ip_address.family() == AF_INET6) {
    s = inet_ntop(AF_INET6, &ip_address.ipv6_address().sin6_addr, buffer.data(),
                  sizeof(buffer));
  } else {
    assert(0 && "Unknown address family");
  }
  out << s;
  return out;
}

//--------------------------------------------------------------------------------------------------
// ToString
//--------------------------------------------------------------------------------------------------
std::string ToString(const IpAddress& ip_address) {
  std::ostringstream oss;
  oss << ip_address;
  return oss.str();
}
}  // namespace lightstep
