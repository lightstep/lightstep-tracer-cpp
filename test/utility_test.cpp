#include "../src/utility.h"
#include <cmath>
#include <limits>

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

TEST_CASE("ReverseEndianness") {
  uint16_t x;
  char* x_data = reinterpret_cast<char*>(&x);
  x_data[0] = 1;
  x_data[1] = 2;
  ReverseEndianness(x);
  CHECK(x_data[0] == 2);
  CHECK(x_data[1] == 1);
}
