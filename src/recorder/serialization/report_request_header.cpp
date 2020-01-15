#include "recorder/serialization/report_request_header.h"

#include "common/protobuf.h"
#include "common/utility.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace lightstep {
std::string WriteReportRequestHeader(
    const LightStepTracerOptions& tracer_options, uint64_t reporter_id) {
  collector::Reporter reporter;
  reporter.set_reporter_id(reporter_id);
  reporter.mutable_tags()->Reserve(
      static_cast<int>(tracer_options.tags.size()));
  for (const auto& tag : tracer_options.tags) {
    *reporter.mutable_tags()->Add() = ToKeyValue(tag.first, tag.second);
  }

  collector::Auth auth;
  auth.set_access_token(tracer_options.access_token);

  std::ostringstream oss;
  {
    google::protobuf::io::OstreamOutputStream zero_copy_stream{&oss};
    google::protobuf::io::CodedOutputStream coded_stream{&zero_copy_stream};

    WriteEmbeddedMessage(
        coded_stream, collector::ReportRequest::kReporterFieldNumber, reporter);
    WriteEmbeddedMessage(coded_stream,
                         collector::ReportRequest::kAuthFieldNumber, auth);
  }

  return oss.str();
}
}  // namespace lightstep
