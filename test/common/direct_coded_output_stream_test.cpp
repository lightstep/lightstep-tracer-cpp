#include "common/direct_coded_output_stream.h"

#include <array>
#include <limits>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("DirectCodedOutputStream") {
  const size_t max_varint64_length = 10;
  std::array<char, max_varint64_length> buffer;

  SECTION("Verify we can correctly serialize numbers") {
    for (int i = 0; i < 1000; ++i) {
      DirectCodedOutputStream stream{
          reinterpret_cast<google::protobuf::uint8*>(buffer.data())};
      auto x = std::numeric_limits<uint64_t>::max() - static_cast<uint64_t>(i);
      stream.WriteBigVarint64(x);
      google::protobuf::io::ArrayInputStream zero_copy_stream{
          static_cast<void*>(buffer.data()),
          static_cast<int>(
              stream.data() -
              reinterpret_cast<google::protobuf::uint8*>(buffer.data()))};
      google::protobuf::io::CodedInputStream input_stream{&zero_copy_stream};
      uint64_t x_prime;
      REQUIRE(input_stream.ReadVarint64(&x_prime));
      REQUIRE(x == x_prime);
    }
  }
}
