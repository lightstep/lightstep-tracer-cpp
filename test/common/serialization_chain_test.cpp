#include "common/serialization_chain.h"

#include <iomanip>
#include <sstream>

#include "test/utility.h"

#include <google/protobuf/io/coded_stream.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SerializationChain") {
  SerializationChain chain;

  std::unique_ptr<google::protobuf::io::CodedOutputStream> stream{
      new google::protobuf::io::CodedOutputStream{&chain}};

  SECTION("An empty chain has no fragments.") {
    stream.reset();
    chain.AddChunkFraming();
    REQUIRE(chain.num_fragments() == 0);
  }

  SECTION("We can write a string smaller than the block size.") {
    stream->WriteString("abc");
    stream.reset();
    chain.AddChunkFraming();
    REQUIRE(chain.num_fragments() == 3);
    REQUIRE(ToString(chain) == "3\r\nabc\r\n");
  }

  SECTION("We can write strings larger than a single block.") {
    std::string s(SerializationChain::BlockSize + 1, 'X');
    stream->WriteString(s);
    stream.reset();
    chain.AddChunkFraming();
    REQUIRE(chain.num_fragments() == 4);
    std::ostringstream oss;
    oss << std::hex << std::uppercase << s.size() << "\r\n" << s << "\r\n";
    REQUIRE(ToString(chain) == oss.str());
  }

  SECTION("We can seek to any byte in the fragment stream.") {
    std::string s(SerializationChain::BlockSize + 2, 'X');
    stream->WriteString(s);
    stream.reset();
    chain.AddChunkFraming();
    std::string serialization = [&] {
      std::ostringstream oss;
      oss << std::hex << std::uppercase << s.size() << "\r\n" << s << "\r\n";
      return oss.str();
    }();
    for (size_t i = 1; i <= serialization.size(); ++i) {
      SECTION("cosumption instance " + std::to_string(i)) {
        Consume({&chain}, i);
        REQUIRE(ToString(chain) == serialization.substr(i));
      }
    }
  }
}
