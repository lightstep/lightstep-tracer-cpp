#include "common/serialization.h"

#include <sstream>
#include <vector>

#include "common/utility.h"

#include "test/common/test.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/message_differencer.h>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

static std::vector<opentracing::Value> GenerateTestValues() {
  return {0,     -123,    123,
          0.0,   123.456, true,
          false, "abc",   opentracing::Values{"abc", 123}};
}

TEST_CASE("Serialization") {
  std::ostringstream oss;
  std::unique_ptr<google::protobuf::io::OstreamOutputStream> output_stream{
      new google::protobuf::io::OstreamOutputStream{&oss}};
  google::protobuf::io::CodedOutputStream coded_stream{output_stream.get()};

  auto finalize = [&] {
    coded_stream.Trim();
    output_stream.reset();
  };

  for (auto& value : GenerateTestValues()) {
    auto key = "abc";
    auto expected_key_value = ToKeyValue(key, value);
    SECTION("We can serialize various KeyValues: " +
            expected_key_value.SerializeAsString()) {
      WriteKeyValue<test::KeyValueTest::kKeyValueFieldNumber>(coded_stream, key,
                                                              value);
      finalize();
      test::KeyValueTest key_value;
      REQUIRE(key_value.ParseFromString(oss.str()));
      REQUIRE(google::protobuf::util::MessageDifferencer::Equals(
          key_value.key_value(), expected_key_value));
    }
  }

  SECTION("We can serialize timestamps") {
    auto now = std::chrono::system_clock::now();
    auto expected_timestamp = ToTimestamp(now);
    WriteTimestamp<test::TimestampTest::kTimestampFieldNumber>(coded_stream,
                                                               now);
    finalize();
    test::TimestampTest timestamp;
    REQUIRE(timestamp.ParseFromString(oss.str()));
    REQUIRE(google::protobuf::util::MessageDifferencer::Equals(
        timestamp.timestamp(), expected_timestamp));
  }
}
