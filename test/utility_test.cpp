#include "../src/utility.h"
#include <arpa/inet.h>
#include <array>
#include <cmath>
#include <limits>

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;
using namespace opentracing;

TEST_CASE("AddressToString") {
  std::array<unsigned char, sizeof(sockaddr_in6)> buffer = {};
  SECTION("ipv4") {
    auto address_in = reinterpret_cast<sockaddr_in*>(buffer.data());
    address_in->sin_family = AF_INET;
    REQUIRE(inet_pton(AF_INET, "54.203.0.1", &address_in->sin_addr) == 1);
    CHECK(AddressToString(*reinterpret_cast<sockaddr*>(buffer.data())) ==
          "54.203.0.1");
  }
  SECTION("ipv6") {
    auto address_in = reinterpret_cast<sockaddr_in6*>(buffer.data());
    address_in->sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8:85a3::8a2e:370:7334", &address_in->sin6_addr);
    CHECK(AddressToString(*reinterpret_cast<sockaddr*>(buffer.data())) ==
          "2001:db8:85a3::8a2e:370:7334");
  }
}

TEST_CASE("Json") {
  SECTION("Arrays jsonify correctly.") {
    auto key_value1 = ToKeyValue("", Values{1});
    CHECK(key_value1.json_value() == "[1]");
    auto key_value2 = ToKeyValue("", Values{1, 2});
    CHECK(key_value2.json_value() == "[1,2]");
  }

  SECTION("Dictionaries jsonify correctly.") {
    auto key_value1 = ToKeyValue("", Dictionary{{"abc", 123}});
    CHECK(key_value1.json_value() == R"({"abc":123})");
  }

  SECTION("Compound aggregates jsonify correctly.") {
    auto key_value1 = ToKeyValue("", Dictionary{{"abc", Values{1, 2}}});
    CHECK(key_value1.json_value() == R"({"abc":[1,2]})");
  }

  SECTION("Special characters in strings are escaped.") {
    auto key_value1 = ToKeyValue("", Values{"\"\n\t\x0f"});
    CHECK(key_value1.json_value() == R"(["\"\n\t\u000f"])");
  }

  SECTION("Doubles jsonify correctly.") {
    auto key_value1 = ToKeyValue("", Values{1.5});
    CHECK(key_value1.json_value() == R"([1.5])");
    auto key_value2 = ToKeyValue("", Values{std::nan(" ")});
    CHECK(key_value2.json_value() == R"(["NaN"])");
    auto key_value3 =
        ToKeyValue("", Values{std::numeric_limits<double>::infinity()});
    CHECK(key_value3.json_value() == R"(["+Inf"])");
    auto key_value4 =
        ToKeyValue("", Values{-std::numeric_limits<double>::infinity()});
    CHECK(key_value4.json_value() == R"(["-Inf"])");
  }

  SECTION("Bools jsonify correctly.") {
    auto key_value1 = ToKeyValue("", Values{true});
    CHECK(key_value1.json_value() == "[true]");
    auto key_value2 = ToKeyValue("", Values{false});
    CHECK(key_value2.json_value() == "[false]");
  }

  SECTION("nullptr jsonifies to null.") {
    auto key_value1 = ToKeyValue("", Values{nullptr});
    CHECK(key_value1.json_value() == "[null]");
  }
}

TEST_CASE("ReverseEndianness") {
  uint16_t x;
  char* x_data = reinterpret_cast<char*>(&x);
  x_data[0] = 1;
  x_data[1] = 2;
  ReverseEndianness(x);
  CHECK(x_data[0] == 2);
  CHECK(x_data[1] == 1);
}

TEST_CASE("hex-integer conversions") {
  char data[16];

  SECTION("Verify hex conversion and back against a range of values.") {
    for (uint64_t x = 0; x < 1000; ++x) {
      CHECK(x == *HexToUint64(Uint64ToHex(x, data)));
      auto y = std::numeric_limits<uint64_t>::max() - x;
      CHECK(y == *HexToUint64(Uint64ToHex(y, data)));
    }
  }

  SECTION("Verify a few special values.") {
    CHECK(Uint64ToHex(0, data) == "0000000000000000");
    CHECK(Uint64ToHex(1, data) == "0000000000000001");
    CHECK(Uint64ToHex(std::numeric_limits<uint64_t>::max(), data) ==
          "FFFFFFFFFFFFFFFF");

    CHECK(*HexToUint64("0") == 0);
    CHECK(*HexToUint64("1") == 1);
    CHECK(*HexToUint64("FFFFFFFFFFFFFFFF") ==
          std::numeric_limits<uint64_t>::max());
  }

  SECTION("Leading or trailing spaces are ignored when converting from hex.") {
    CHECK(*HexToUint64("  \tabc") == 0xabc);
    CHECK(*HexToUint64("abc  \t") == 0xabc);
    CHECK(*HexToUint64("  \tabc  \t") == 0xabc);
  }

  SECTION("Hex conversion works with both upper and lower case digits.") {
    CHECK(*HexToUint64("ABCDEF") == 0xABCDEF);
    CHECK(*HexToUint64("abcdef") == 0xABCDEF);
  }

  SECTION("Hex conversion with an empty string gives an error.") {
    CHECK(!HexToUint64(""));
    CHECK(!HexToUint64("  "));
  }

  SECTION(
      "Hex conversion of a number bigger than "
      "std::numeric_limits<uint64_t>::max() gives an error.") {
    CHECK(!HexToUint64("1123456789ABCDEF1"));
  }

  SECTION(
      "Hex conversion of a number within valid limits but with leading zeros "
      "past 16 digits is successful.") {
    CHECK(HexToUint64("0123456789ABCDEF1"));
  }

  SECTION("Hex conversion with invalid digits gives an error.") {
    CHECK(!HexToUint64("abcHef"));
  }
}
