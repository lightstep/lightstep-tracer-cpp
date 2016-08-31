// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: collector.proto

#include "collector.pb.h"
#include "collector.grpc.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>
namespace lightstep {
namespace collector {

static const char* CollectorService_method_names[] = {
  "/lightstep.collector.CollectorService/Report",
};

std::unique_ptr< CollectorService::Stub> CollectorService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  std::unique_ptr< CollectorService::Stub> stub(new CollectorService::Stub(channel));
  return stub;
}

CollectorService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_Report_(CollectorService_method_names[0], ::grpc::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status CollectorService::Stub::Report(::grpc::ClientContext* context, const ::lightstep::collector::ReportRequest& request, ::lightstep::collector::ReportResponse* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_Report_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::lightstep::collector::ReportResponse>* CollectorService::Stub::AsyncReportRaw(::grpc::ClientContext* context, const ::lightstep::collector::ReportRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::lightstep::collector::ReportResponse>(channel_.get(), cq, rpcmethod_Report_, context, request);
}

CollectorService::Service::Service() {
  (void)CollectorService_method_names;
  AddMethod(new ::grpc::RpcServiceMethod(
      CollectorService_method_names[0],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< CollectorService::Service, ::lightstep::collector::ReportRequest, ::lightstep::collector::ReportResponse>(
          std::mem_fn(&CollectorService::Service::Report), this)));
}

CollectorService::Service::~Service() {
}

::grpc::Status CollectorService::Service::Report(::grpc::ServerContext* context, const ::lightstep::collector::ReportRequest* request, ::lightstep::collector::ReportResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace lightstep
}  // namespace collector

