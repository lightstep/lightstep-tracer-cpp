#include "../src/utility.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>
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

  SECTION("Verify that special characters in strings are escaped.") {
    auto key_value1 = ToKeyValue("", Values{"\"\n\t"});
    CHECK(key_value1.json_value() == R"(["\"\n\t"])");
  }
}
