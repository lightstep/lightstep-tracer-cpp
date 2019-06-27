#include "common/circular_buffer2.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("CircularBuffer") {
  CircularBuffer2<int> buffer{100};
  (void)buffer;
}
