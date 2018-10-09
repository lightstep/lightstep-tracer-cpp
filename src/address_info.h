#pragma once

#include <netdb.h>
#include <stdexcept>

namespace lightstep {
class AddressInfoList {
 public:
  explicit AddressInfoList(addrinfo* head) noexcept;

  ~AddressInfoList();

  AddressInfoList(AddressInfoList&) = delete;
  AddressInfoList(AddressInfoList&& other) noexcept;

  AddressInfoList& operator=(AddressInfoList& other) = delete;
  AddressInfoList& operator=(AddressInfoList&& other) & noexcept;

  bool empty() const { return head_ == nullptr; }

  template <class F>
  bool forEachAddressInfo(F f) const {
    addrinfo* node = head_;
    while (node != nullptr) {
      if (!f(*node)) {
        return false;
      }
      node = node->ai_next;
    }
    return true;
  }

 private:
  addrinfo* head_;
};

class AddressInfoFailure : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

AddressInfoList GetAddressInfo(const char* hostname);

AddressInfoList GetAddressInfo(const char* hostname, uint16_t port);
}  // namespace lightstep
