#include "common/hex_conversion.h"

#include <limits>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;
using namespace opentracing;

template <class F>
static void Generate128BitTestCases(uint64_t x, F f) {
  auto max = std::numeric_limits<uint64_t>::max();
  f(x, x);
  f(0, x);
  f(x, 0);
  f(max, x);
  f(x, max);
  f(max - x, x);
  f(x, max - x);
  f(max - x, max - x);
}

TEST_CASE("hex-integer conversions (8-bit)") {
  char data[Num8BitHexDigits];

  SECTION("Verify hex conversion and back against a range of values.") {
    for (uint8_t x = 0; x < std::numeric_limits<uint8_t>::max(); ++x) {
      {
        REQUIRE(x == *NormalizedHexToUint8(Uint8ToHex(x, data)));
        auto y = std::numeric_limits<uint8_t>::max() - x;
        REQUIRE(y == *NormalizedHexToUint8(Uint8ToHex(y, data)));
      }
    }
  }
}

TEST_CASE("hex-integer conversions (64-bit)") {
  char data[Num64BitHexDigits];

  SECTION("Verify hex conversion and back against a range of values.") {
    for (uint32_t x = 0; x < 1000; ++x) {
      {
        REQUIRE(x == *HexToUint64(Uint64ToHex(x, data)));
        auto y = std::numeric_limits<uint64_t>::max() - x;
        REQUIRE(y == *HexToUint64(Uint64ToHex(y, data)));
      }
      {
        REQUIRE(x == *HexToUint64(Uint32ToHex(x, data)));
        auto y = std::numeric_limits<uint32_t>::max() - x;
        REQUIRE(y == *HexToUint64(Uint64ToHex(y, data)));
      }
    }
  }

  SECTION("Verify a few special values.") {
    REQUIRE(Uint64ToHex(0, data) == "0000000000000000");
    REQUIRE(Uint64ToHex(1, data) == "0000000000000001");
    REQUIRE(Uint64ToHex(std::numeric_limits<uint64_t>::max(), data) ==
            "ffffffffffffffff");

    REQUIRE(*HexToUint64("0") == 0);
    REQUIRE(*HexToUint64("1") == 1);
    REQUIRE(*HexToUint64("ffffffffffffffff") ==
            std::numeric_limits<uint64_t>::max());
  }

  SECTION("Leading or trailing spaces are ignored when converting from hex.") {
    REQUIRE(*HexToUint64("  \tabc") == 0xabc);
    REQUIRE(*HexToUint64("abc  \t") == 0xabc);
    REQUIRE(*HexToUint64("  \tabc  \t") == 0xabc);
  }

  SECTION("Hex conversion works with both upper and lower case digits.") {
    REQUIRE(*HexToUint64("ABCDEF") == 0xABCDEF);
    REQUIRE(*HexToUint64("abcdef") == 0xABCDEF);
  }

  SECTION("Hex conversion with an empty string gives an error.") {
    REQUIRE(!HexToUint64(""));
    REQUIRE(!HexToUint64("  "));
  }

  SECTION(
      "Hex conversion of a number bigger than "
      "std::numeric_limits<uint64_t>::max() gives an error.") {
    REQUIRE(!HexToUint64("1123456789ABCDEF1"));
  }

  SECTION(
      "Hex conversion of a number within valid limits but with leading zeros "
      "past 16 digits is successful.") {
    REQUIRE(HexToUint64("0123456789ABCDEF1"));
  }

  SECTION("Hex conversion with invalid digits gives an error.") {
    REQUIRE(!HexToUint64("abcHef"));
  }
}

TEST_CASE("hex-integer conversions (128-bit)") {
  HexSerializer serializer;

  SECTION("We can serialize 128-bit integers") {
    REQUIRE(serializer.Uint128ToHex(0, 1) == "0000000000000001");
    REQUIRE(serializer.Uint128ToHex(1, 0) ==
            "0000000000000001"
            "0000000000000000");
    REQUIRE(serializer.Uint128ToHex(std::numeric_limits<uint64_t>::max(),
                                    std::numeric_limits<uint64_t>::max()) ==
            "ffffffffffffffff"
            "ffffffffffffffff");
  }

  SECTION("Verify hex conversion and back against a range of values.") {
    for (uint32_t x = 0; x < 1000; ++x) {
      Generate128BitTestCases(x, [&](uint64_t x_high, uint64_t x_low) {
        auto s = serializer.Uint128ToHex(x_high, x_low);
        uint64_t x_high_prime;
        uint64_t x_low_prime;
        auto was_successful = HexToUint128(s, x_high_prime, x_low_prime);
        REQUIRE(was_successful);
        REQUIRE(x_high_prime == x_high);
        REQUIRE(x_low_prime == x_low_prime);
      });
    }
  }

  SECTION("Verify hex conversion fails on bad digits cases") {
    uint64_t x_high;
    uint64_t x_low;
    auto s1 =
        "FFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFx";
    REQUIRE(!HexToUint128(s1, x_high, x_low));

    auto s2 =
        "FFFFFFFFFFFFFFFx"
        "FFFFFFFFFFFFFFFF";
    REQUIRE(!HexToUint128(s2, x_high, x_low));
  }

  SECTION("Verify hex conversion fails on overflow") {
    uint64_t x_high;
    uint64_t x_low;
    auto s1 =
        "FFFFFFFFFFFFFFFF0"
        "FFFFFFFFFFFFFFFF";
    REQUIRE(!HexToUint128(s1, x_high, x_low));
  }

  SECTION("Hex conversion fails on an empty string") {
    uint64_t x_high;
    uint64_t x_low;
    REQUIRE(!HexToUint128("", x_high, x_low));
  }

  SECTION(
      "Hex conversion of a number within valid limits but with leading zeros "
      "past 32 digits is successful.") {
    uint64_t x_high;
    uint64_t x_low;
    auto s1 =
        "000FFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFF";
    REQUIRE(HexToUint128(s1, x_high, x_low));
    REQUIRE(x_high == std::numeric_limits<uint64_t>::max());
    REQUIRE(x_low == std::numeric_limits<uint64_t>::max());
  }

  SECTION(
      "Hex conversion of a number 128-bit number with the high part all zero "
      "is successful") {
    uint64_t x_high;
    uint64_t x_low;
    auto s1 =
        "0000000000000000"
        "FFFFFFFFFFFFFFFF";
    REQUIRE(HexToUint128(s1, x_high, x_low));
    REQUIRE(x_high == 0);
    REQUIRE(x_low == std::numeric_limits<uint64_t>::max());
  }
}
