#include "common/protobuf.h"

#include <sstream>

#include "lightstep-tracer-common/collector.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/message_differencer.h>
#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("Protobuf") {
  collector::ReportRequest report;
  report.mutable_auth()->set_access_token("abc");
  std::ostringstream oss;
  {
    google::protobuf::io::OstreamOutputStream zero_copy_stream{&oss};
    google::protobuf::io::CodedOutputStream coded_stream{&zero_copy_stream};
    WriteEmbeddedMessage(coded_stream,
                         collector::ReportRequest::kAuthFieldNumber,
                         report.auth());
  }
  auto serialization = oss.str();

  SECTION(
      "WriteEmbeddedMessage writes messages that can be read out by "
      "protobuf.") {
    collector::ReportRequest deserialized_report;
    REQUIRE(deserialized_report.ParseFromString(serialization));
    REQUIRE(google::protobuf::util::MessageDifferencer::Equals(
        report, deserialized_report));
  }

  SECTION(
      "ComputeEmbeddedMessageSerializationSize can be used to compute the "
      "serialization size of an embedded message.") {
    REQUIRE(serialization.size() ==
            ComputeEmbeddedMessageSerializationSize(
                collector::ReportRequest::kAuthFieldNumber, report.auth()));
  }
}
