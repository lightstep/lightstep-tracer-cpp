#include "grpc_dummy_satellite.h"

#include "lightstep-tracer-common/collector.grpc.pb.h"

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc/grpc.h>

#include <iostream>
using namespace lightstep;

namespace lightstep {
void GrpcDummySatellite::Reserve(size_t num_span_ids) {
  std::unique_lock<std::mutex> lock{mutex_};
  span_ids_.reserve(num_span_ids);
}

std::vector<uint64_t> GrpcDummySatellite::span_ids() const {
  std::unique_lock<std::mutex> lock{mutex_};
  return span_ids_;
}

grpc::Status GrpcDummySatellite::Report(
    grpc::ServerContext* context,
    const lightstep::collector::ReportRequest* request,
    lightstep::collector::ReportResponse* response) {
  std::unique_lock<std::mutex> lock{mutex_};
  for (auto& span : request->spans()) {
    span_ids_.push_back(span.span_context().span_id());
  }
  return grpc::Status::OK;
}

void GrpcDummySatellite::Run() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_ = builder.BuildAndStart();
}
}  // namespace lightstep
