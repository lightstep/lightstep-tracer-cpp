#pragma once

#include <cassert>
#include <iosfwd>
#include <string>

// for most socket things
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#include "winsock2.h"
#endif

// for sockaddr_in6
#include "ws2ipdef.h"

// we have to define this manually because winsock2.h sucks
typedef unsigned int sa_family_t;

namespace lightstep {
/**
 * Wraps the system objects for an ip address.
 */
class IpAddress {
 public:
  IpAddress() noexcept;

  IpAddress(const in_addr& addr) noexcept;

  IpAddress(const in6_addr& addr) noexcept;

  IpAddress(const sockaddr& addr) noexcept;

  explicit IpAddress(const char* s);

  IpAddress(const char* s, uint16_t port);

  /**
   * @return the system sockaddr object associated with this ip address.
   */
  const sockaddr& addr() const noexcept {
    return reinterpret_cast<const sockaddr&>(data_);
  }

  /**
   * @return the family of the ip address.
   */
  sa_family_t family() const noexcept { return data_.ss_family; }

  /**
   * @returns for an ipv4 address, returns the system sockaddr_in object.
   */
  const sockaddr_in& ipv4_address() const noexcept {
    assert(family() == AF_INET);
    return reinterpret_cast<const sockaddr_in&>(data_);
  }

  /**
   * @returns for an ipv6 address, returns the system sockaddr_in6 object.
   */
  const sockaddr_in6& ipv6_address() const noexcept {
    assert(family() == AF_INET6);
    return reinterpret_cast<const sockaddr_in6&>(data_);
  }

  /**
   * Sets the port for the ip address.
   * @param port supplies the port to assign.
   */
  void set_port(uint16_t port) noexcept;

 private:
  sockaddr_storage data_{};
};

bool operator==(const IpAddress& lhs, const IpAddress& rhs) noexcept;

inline bool operator!=(const IpAddress& lhs, const IpAddress& rhs) noexcept {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const IpAddress& ip_address);

/**
 * Obtains a readable string for the given ip address.
 * @param ip_address supplies the address to obtain a string for.
 * @return a readable string for the ip address.
 */
std::string ToString(const IpAddress& ip_address);
}  // namespace lightstep
