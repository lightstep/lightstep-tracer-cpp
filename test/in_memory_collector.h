#pragma once

#include <mutex>
#include <vector>
#include "lightstep-tracer-common/collector.grpc.pb.h"

namespace lightstep {
class InMemoryCollector
    : public lightstep::collector::CollectorService::Service {
 public:
  grpc::Status Report(grpc::ServerContext* context,
                      const lightstep::collector::ReportRequest* request,
                      lightstep::collector::ReportResponse* response) override;

  std::vector<collector::Span> spans() const;

 private:
  mutable std::mutex mutex_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
