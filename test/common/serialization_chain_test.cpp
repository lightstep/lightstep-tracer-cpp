#include "common/serialization_chain.h"

#include "test/utility.h"

#include <google/protobuf/io/coded_stream.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SerializationChain") {
  SerializationChain chain;
  google::protobuf::io::CodedOutputStream stream(&chain);

  SECTION("We can write a string smaller than the block size") {
    stream.WriteString("abc");
    chain.AddChunkFraming();
    REQUIRE(ToString(chain) == "3\r\nabc\r\n");
  }
}
