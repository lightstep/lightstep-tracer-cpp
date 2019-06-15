#include "common/serialization.h"

#include <sstream>

#include "test/common/test.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("Serialization") {
  std::stringstream sstream;
  std::unique_ptr<google::protobuf::io::OstreamOutputStream> output_stream{
      new google::protobuf::io::OstreamOutputStream{&sstream}};
  google::protobuf::io::IstreamInputStream input_stream{&sstream};
  google::protobuf::io::CodedOutputStream coded_stream{output_stream.get()};

  auto finalize = [&] {
    coded_stream.Trim();
    output_stream.reset();
  };

  SECTION("We can serialize KeyValues") {
    opentracing::Value value{123};
    SerializeKeyValue<0>(coded_stream, "abc", value);
    finalize();
    REQUIRE(sstream.str().size() > 0);
  }
}
