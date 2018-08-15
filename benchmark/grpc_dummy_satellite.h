#pragma once

#include "dummy_satellite.h"

#include "lightstep-tracer-common/collector.grpc.pb.h"

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc/grpc.h>

#include <memory>
#include <mutex>

namespace lightstep {
class GrpcDummySatellite final : public DummySatellite,
                                 public collector::CollectorService::Service {
 public:
 explicit GrpcDummySatellite(const char* address) : address_{address} {}

  void Run();

  std::vector<uint64_t> span_ids() const override;

  void Reserve(size_t num_span_ids) override;
 private:
  const char* address_;
  std::unique_ptr<grpc::Server> server_;

  mutable std::mutex mutex_;
  std::vector<uint64_t> span_ids_;

  grpc::Status Report(grpc::ServerContext* context,
                      const lightstep::collector::ReportRequest* request,
                      lightstep::collector::ReportResponse* response) override;
};
} // namespace lightstep
