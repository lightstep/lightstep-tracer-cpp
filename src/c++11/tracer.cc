#include <atomic>
#include <iostream>
#include <memory>

#include "lightstep/impl.h"
#include "lightstep/tracer.h"

namespace lightstep {
namespace {

std::atomic<ImplPtr*> global_tracer;

}  // namespace

BuiltinCarrierFormat BuiltinCarrierFormat::BinaryCarrier{BuiltinCarrierFormat::Binary};
BuiltinCarrierFormat BuiltinCarrierFormat::TextMapCarrier{BuiltinCarrierFormat::TextMap};
BuiltinCarrierFormat BuiltinCarrierFormat::HTTPHeadersCarrier{BuiltinCarrierFormat::HTTPHeaders};

Tracer Tracer::Global() {
  ImplPtr *ptr = global_tracer.load();
  if (ptr == nullptr) {
    return Tracer(nullptr);
  }
  // Note: there is an intentional race here.  InitGlobal releases the
  // shared pointer reference without synchronization.
  return Tracer(*ptr);
}

Tracer Tracer::InitGlobal(Tracer newt) {
  ImplPtr *newptr = new ImplPtr(newt.impl_);
  if (auto oldptr = global_tracer.exchange(newptr)) {
    delete oldptr;
  }
  return Tracer(*newptr);
}

Span Tracer::StartSpan(const std::string& operation_name,
		       SpanStartOptions opts) const {
  if (!impl_) return Span();
  return Span(impl_->StartSpan(impl_, operation_name, opts));
}

bool Tracer::Inject(SpanContext sc, const CarrierFormat &format, const CarrierWriter &writer) {
  return impl_->inject(sc, format, writer);
}

SpanContext Tracer::Extract(const CarrierFormat& format, const CarrierReader& reader) {
  return impl_->extract(format, reader);
}

SpanReference ChildOf(const SpanContext& ctx) {
  return SpanReference(ChildOfRef, ctx);
}

SpanReference FollowsFrom(const SpanContext& ctx) {
  return SpanReference(FollowsFromRef, ctx);
}

Tracer NewUserDefinedTransportLightStepTracer(const TracerOptions& topts, RecorderFactory rf) {
  ImplPtr impl(new TracerImpl(topts));
  impl->set_recorder(rf(*impl));
  return Tracer(impl);
}

ReportBuilder::ReportBuilder(const TracerImpl &impl)
  : reset_next_(true) {
  // TODO Fill in any core internal_metrics.
  collector::Tracer *tracer = preamble_.mutable_tracer();
  for (const auto& tt : impl.options().tracer_attributes) {
    *tracer->mutable_tags()->Add() = util::make_kv(tt.first, tt.second);

    if (tt.first == TracerIDKey) {
      tracer->set_tracer_id(util::stringToUint64(tt.second));
    }
  }
  preamble_.mutable_auth()->set_access_token(impl.access_token());
}

}  // namespace lightstep
