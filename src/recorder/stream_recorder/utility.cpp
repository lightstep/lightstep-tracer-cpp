#include "recorder/stream_recorder/utility.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

#include "common/protobuf.h"
#include "common/utility.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <strings.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// SeparateEndpoints
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<const char*>, std::vector<std::pair<int, uint16_t>>>
SeparateEndpoints(
    const std::vector<std::pair<std::string, uint16_t>>& endpoints) {
  std::vector<const char*> hosts;
  hosts.reserve(endpoints.size());
  for (auto& endpoint : endpoints) {
    hosts.emplace_back(endpoint.first.c_str());
  }
  auto less_than = [](const char* lhs, const char* rhs) {
    return strcasecmp(lhs, rhs) < 0;
  };
  auto equal_to = [](const char* lhs, const char* rhs) {
    return strcasecmp(lhs, rhs) == 0;
  };
  std::sort(hosts.begin(), hosts.end(), less_than);
  hosts.erase(std::unique(hosts.begin(), hosts.end(), equal_to), hosts.end());

  std::vector<std::pair<int, uint16_t>> indexed_endpoints;
  indexed_endpoints.reserve(endpoints.size());
  for (auto& endpoint : endpoints) {
    auto iter = std::lower_bound(hosts.begin(), hosts.end(),
                                 endpoint.first.c_str(), less_than);
    assert(iter != hosts.end());
    indexed_endpoints.emplace_back(
        static_cast<int>(std::distance(hosts.begin(), iter)), endpoint.second);
  }
  return {std::move(hosts), std::move(indexed_endpoints)};
}

//--------------------------------------------------------------------------------------------------
// WriteStreamHeaderCommonFragment
//--------------------------------------------------------------------------------------------------
std::string WriteStreamHeaderCommonFragment(
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

//--------------------------------------------------------------------------------------------------
// Contains
//--------------------------------------------------------------------------------------------------
bool Contains(const char* data, size_t size, const char* ptr) noexcept {
  if (data == nullptr) {
    return false;
  }
  if (ptr == nullptr) {
    return false;
  }
  auto delta = std::distance(data, ptr);
  if (delta < 0) {
    return false;
  }
  return static_cast<size_t>(delta) < size;
}

//--------------------------------------------------------------------------------------------------
// SetSpanFragment
//--------------------------------------------------------------------------------------------------
void SetSpanFragment(FragmentArrayInputStream& stream,
                     const CircularBufferConstPlacement& placement,
                     const char* position) {
  if (Contains(placement.data1, placement.size1, position)) {
    stream.Add(Fragment{static_cast<void*>(const_cast<char*>(position)),
                        static_cast<int>(std::distance(
                            position, placement.data1 + placement.size1))});
    if (placement.size2 > 0) {
      stream.Add(
          Fragment{static_cast<void*>(const_cast<char*>(placement.data2)),
                   static_cast<int>(placement.size2)});
    }
    return;
  }
  assert(Contains(placement.data2, placement.size2, position));
  stream.Add(Fragment{static_cast<void*>(const_cast<char*>(position)),
                      static_cast<int>(std::distance(
                          position, placement.data2 + placement.size2))});
}
}  // namespace lightstep
