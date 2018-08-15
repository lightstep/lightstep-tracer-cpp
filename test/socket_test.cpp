#include "../src/socket.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>

using namespace lightstep;

TEST_CASE("Socket") {
  SECTION("Moving from a socket sets an invalid file descriptor") {
    Socket s1;
    auto file_descriptor = s1.file_descriptor();
    Socket s2{std::move(s1)};
    CHECK(s2.file_descriptor() == file_descriptor);
    CHECK(s1.file_descriptor() == -1);
  }
}
