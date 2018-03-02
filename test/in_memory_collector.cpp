#include "in_memory_collector.h"

namespace lightstep {
grpc::Status InMemoryCollector::Report(
    grpc::ServerContext* /*context*/,
    const lightstep::collector::ReportRequest* request,
    lightstep::collector::ReportResponse* /*response*/) {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (auto& span : request->spans()) {
    spans_.emplace_back(span);
  }
  return grpc::Status::OK;
}

std::vector<collector::Span> InMemoryCollector::spans() const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return spans_;
}
}  // namespace lightstep
