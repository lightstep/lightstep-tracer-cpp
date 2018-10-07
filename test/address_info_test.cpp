#include "../src/address_info.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>

using namespace lightstep;

TEST_CASE("AddressInfo") {
  auto address_info_list = GetAddressInfo("localhost");
  CHECK(!address_info_list.empty());

  SECTION("We can iterate over the available addresses.") {
    int num_addresses = 0;
    auto result = address_info_list.forEachAddressInfo(
        [&](const addrinfo& /*address_info*/) {
          ++num_addresses;
          return true;
        });
    CHECK(result);
    CHECK(num_addresses > 0);
  }

  SECTION("We can bail out of iterating over addresses by returning false.") {
    int num_addresses = 0;
    auto result = address_info_list.forEachAddressInfo(
        [&](const addrinfo& /*address_info*/) {
          ++num_addresses;
          return false;
        });
    CHECK(!result);
    CHECK(num_addresses == 1);
  }

  SECTION("We can move construct AddressInfoList.") {
    AddressInfoList address_info_list2{std::move(address_info_list)};
    CHECK(!address_info_list2.empty());
    CHECK(address_info_list.empty());
  }

  SECTION("We can move assign AddressInfoList.") {
    AddressInfoList address_info_list2 = GetAddressInfo("localhost");
    address_info_list2 = std::move(address_info_list);
    CHECK(!address_info_list2.empty());
    CHECK(address_info_list.empty());
  }

  SECTION("Looking up an invalid address throws an exception.") {
    CHECK_THROWS_AS(GetAddressInfo("abc123%$"), AddressInfoFailure);
    CHECK_THROWS_AS(GetAddressInfo("kthownv.no.exist"), AddressInfoFailure);
  }
}
