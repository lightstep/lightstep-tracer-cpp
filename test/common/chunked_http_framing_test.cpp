#include "common/chunked_http_framing.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("ChunkedHttpFraming") {
  std::string header_serialization(ChunkedHttpMaxHeaderSize + 1, ' ');
  WriteHttpChunkHeader(&header_serialization[0], header_serialization.size(),
                       10);
  REQUIRE(header_serialization == " 0000000a\r\n");
}
