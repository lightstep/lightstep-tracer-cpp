#include "grpc_dummy_satellite.h"

#include "lightstep-tracer-common/collector.grpc.pb.h"

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc/grpc.h>

#include <iostream>
using namespace lightstep;

namespace lightstep {
//------------------------------------------------------------------------------
// Reserve
//------------------------------------------------------------------------------
void GrpcDummySatellite::Reserve(size_t num_span_ids) {
  std::unique_lock<std::mutex> lock{mutex_};
  span_ids_.reserve(num_span_ids);
}

//------------------------------------------------------------------------------
// span_ids
//------------------------------------------------------------------------------
std::vector<uint64_t> GrpcDummySatellite::span_ids() const {
  std::unique_lock<std::mutex> lock{mutex_};
  return span_ids_;
}

//------------------------------------------------------------------------------
// Report
//------------------------------------------------------------------------------
grpc::Status GrpcDummySatellite::Report(
    grpc::ServerContext* /*context*/,
    const lightstep::collector::ReportRequest* request,
    lightstep::collector::ReportResponse* /*response*/) {
  std::unique_lock<std::mutex> lock{mutex_};
  for (auto& span : request->spans()) {
    span_ids_.push_back(span.span_context().span_id());
  }
  auto& metrics = request->internal_metrics();
  for (auto& count : metrics.counts()) {
    if (count.name() == "spans.dropped") {
      num_dropped_spans_ += count.int_value();
    }
  }
  return grpc::Status::OK;
}

//------------------------------------------------------------------------------
// Run
//------------------------------------------------------------------------------
void GrpcDummySatellite::Run() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_ = builder.BuildAndStart();
}

//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void GrpcDummySatellite::Close() {}
}  // namespace lightstep
