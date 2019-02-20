#include "network/socket.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

TEST_CASE("Socket") {
  SECTION("Socket can be move constructed.") {
    Socket s1;
    auto file_descriptor = s1.file_descriptor();
    Socket s2{std::move(s1)};
    CHECK(s2.file_descriptor() == file_descriptor);
    CHECK(s1.file_descriptor() == -1);
  }

  SECTION("Socket can be move assigned.") {
    Socket s1, s2;
    auto file_descriptor = s1.file_descriptor();
    s2 = std::move(s1);
    REQUIRE(s2.file_descriptor() == file_descriptor);
    REQUIRE(s1.file_descriptor() == -1);
  }

  SECTION("We can change options on a socket.") {
    Socket s1;
    s1.SetNonblocking();
    s1.SetReuseAddress();
  }
}
