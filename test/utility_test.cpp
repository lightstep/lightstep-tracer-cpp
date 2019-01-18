#include "../src/utility.h"
#include <cmath>
#include <limits>
#include "../src/bipart_memory_stream.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;
using namespace opentracing;

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

TEST_CASE(
    "ReadChunkHeader reads the header from an http/1.1 streaming chunk.") {
  SECTION("We can read the chunk header from contiguous memory.") {
    BipartMemoryInputStream stream{"A\r\n", 3, nullptr, 0};
    size_t chunk_size;
    REQUIRE(ReadChunkHeader(stream, chunk_size));
    REQUIRE(chunk_size == 10);
    REQUIRE(stream.ByteCount() == 3);
  }

  SECTION(
      "We can read the chunk header from contiguous memory that spans multiple "
      "digits.") {
    BipartMemoryInputStream stream{"A1\r\n", 4, nullptr, 0};
    size_t chunk_size;
    REQUIRE(ReadChunkHeader(stream, chunk_size));
    REQUIRE(chunk_size == 161);
    REQUIRE(stream.ByteCount() == 4);
  }

  SECTION("Only the chunk header is read from the stream.") {
    BipartMemoryInputStream stream{"A1\r\nabc", 7, nullptr, 0};
    size_t chunk_size;
    REQUIRE(ReadChunkHeader(stream, chunk_size));
    REQUIRE(chunk_size == 161);
    REQUIRE(stream.ByteCount() == 4);
  }

  SECTION(
      "We can read a header that's split across separate parts of contiguous "
      "memory.") {
    BipartMemoryInputStream stream{"A", 1, "1\r\n", 3};
    size_t chunk_size;
    REQUIRE(ReadChunkHeader(stream, chunk_size));
    REQUIRE(chunk_size == 161);
    REQUIRE(stream.ByteCount() == 4);
  }

  SECTION("ReadChunkHeader fails if there's no carriage return.") {
    BipartMemoryInputStream stream{"A", 1, "123", 3};
    size_t chunk_size;
    REQUIRE(!ReadChunkHeader(stream, chunk_size));
  }

  SECTION("ReadChunkHeader fails if there are invalid characters.") {
    BipartMemoryInputStream stream{"AX\r\n", 4, nullptr, 0};
    size_t chunk_size;
    REQUIRE(!ReadChunkHeader(stream, chunk_size));
  }

  SECTION(
      "ReadChunkHeader fails if the number overflows an unsigned 64-bit "
      "integer.") {
    BipartMemoryInputStream stream{"0123456789ABCDEF0\r\n", 19, nullptr, 0};
    size_t chunk_size;
    REQUIRE(!ReadChunkHeader(stream, chunk_size));
  }

  SECTION("ReadChunkHeader fails if two bytes don't follow the hex number.") {
    BipartMemoryInputStream stream{"A0\r", 3, nullptr, 0};
    size_t chunk_size;
    REQUIRE(!ReadChunkHeader(stream, chunk_size));
  }
}
