#include "../src/packet_header.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;

TEST_CASE("PacketHeader") {
  char buffer[PacketHeader::size];
  PacketHeader header1{PacketType::Span, 156};
  header1.serialize(buffer);

  PacketHeader header2{buffer};

  CHECK(header2.type() == PacketType::Span);
  CHECK(header2.body_size() == 156);
}
