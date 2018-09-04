#include "../src/utility.h"
#include <cmath>
#include <limits>
#include <sstream>

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

TEST_CASE("Uint64ToHex") {
  auto f = [](const std::string& s) {
    std::istringstream stream{s};
    uint64_t x;
    stream >> std::setw(16) >> std::hex >> x;
    return x;
  };
  char data[16];

  SECTION("Verify hex conversion and back against a range of values.") {
    for (uint64_t x = 0; x < 1000; ++x) {
      CHECK(x == f(Uint64ToHex(x, data)));
    }
  }

  SECTION("Verify hex conversion against the maximum possible value.") {
    auto x = std::numeric_limits<uint64_t>::max();
    CHECK(x == f(Uint64ToHex(x, data)));
  }
}
