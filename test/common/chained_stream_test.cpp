#include "common/chained_stream.h"

#include <iomanip>
#include <random>

#include "test/utility.h"

#include <google/protobuf/io/coded_stream.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("ChainedStream") {
  ChainedStream chain;

  std::unique_ptr<google::protobuf::io::CodedOutputStream> stream{
      new google::protobuf::io::CodedOutputStream{&chain}};

  SECTION("An empty chain has no fragments.") {
    stream.reset();
    chain.CloseOutput();
    REQUIRE(chain.num_fragments() == 0);
  }

  SECTION("We can write a string smaller than the block size.") {
    stream->WriteString("abc");
    stream.reset();
    chain.CloseOutput();
    REQUIRE(chain.num_fragments() == 1);
    REQUIRE(ToString(chain) == "abc");
  }

  SECTION("We can write strings larger than a single block.") {
    std::string s(ChainedStream::BlockSize + 1, 'X');
    stream->WriteString(s);
    stream.reset();
    chain.CloseOutput();
    REQUIRE(chain.num_fragments() == 2);
    REQUIRE(ToString(chain) == s);
  }
}
