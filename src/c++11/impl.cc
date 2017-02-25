#include <string.h>
#include <functional>
#include <random>
#include <sstream>
#include <iomanip>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "lightstep/envoy.h"
#include "lightstep/impl.h"
#include "lightstep/options.h"
#include "lightstep/tracer.h"
#include "lightstep/util.h"

// NOTE that protobuf doesn't have C++11 move constructors. The code
// below does somewhat more allocation and copying than neccesary as a
// result.

namespace lightstep {

const char ReporterIDKey[] = "lightstep.guid";

namespace {

const char TraceKeyPrefix[] = "join:";
const char ParentSpanGUIDKey[] = "parent_span_guid";
const char UndefinedSpanName[] = "undefined";
const char ComponentNameKey[] = "lightstep.component_name";
const char PlatformNameKey[] = "lightstep.tracer_platform";
const char VersionNameKey[] = "lightstep.tracer_version";

#define PREFIX_TRACER_STATE "ot-tracer-"
// Note: these constants are a convention of the OpenTracing basictracers.
const char PrefixTracerState[] = PREFIX_TRACER_STATE;
const char PrefixBaggage[] = "ot-baggage-";

const char FieldCount = 3;
const char FieldNameTraceID[] = PREFIX_TRACER_STATE "traceid";
const char FieldNameSpanID[] = PREFIX_TRACER_STATE "spanid";
const char FieldNameSampled[] = PREFIX_TRACER_STATE "sampled";
#undef PREFIX_TRACER_STATE

typedef google::protobuf::Map< ::std::string, ::std::string > StringMap;

std::string uint64ToHex(uint64_t u) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(16) << std::hex << u;
  return ss.str();
}

} // namespace

TracerImpl::TracerImpl(const TracerOptions& options_in)
  : options_(options_in),
    rand_source_(std::random_device()()),
    tracer_start_time_(Clock::now()) {

  reporter_id_ = GetOneId();

  if (options_.tracer_attributes.find(ComponentNameKey) == options_.tracer_attributes.end()) {
    options_.tracer_attributes.emplace(std::make_pair(ComponentNameKey, util::program_name()));
  }
  // Note: drop the ReporterIDKey, this is copied from reporter_id_.
  auto guid_lookup = options_.tracer_attributes.find(ReporterIDKey);
  if (guid_lookup != options_.tracer_attributes.end()) {
    options_.tracer_attributes.erase(guid_lookup);
  }

  options_.tracer_attributes.emplace(std::make_pair(PlatformNameKey, "c++11"));
  options_.tracer_attributes.emplace(std::make_pair(VersionNameKey, PACKAGE_VERSION));
}

void TracerImpl::GetTwoIds(uint64_t *a, uint64_t *b) {
  if (options_.guid_generator != nullptr) {
    *a = options_.guid_generator();
    *b = options_.guid_generator();
  } else {
    MutexLock l(mutex_);
    *a = rand_source_();  
    *b = rand_source_();
  }
}

uint64_t TracerImpl::GetOneId() {
  if (options_.guid_generator != nullptr) {
    return options_.guid_generator();
  } else {
    MutexLock l(mutex_);
    return rand_source_();
  }
}

void TracerImpl::set_recorder(std::unique_ptr<Recorder> recorder) {
  recorder_ = std::shared_ptr<Recorder>(std::move(recorder));
}

std::unique_ptr<SpanImpl> TracerImpl::StartSpan(std::shared_ptr<TracerImpl> tracer,
						const std::string& operation_name,
						SpanStartOptions opts) {
  std::unique_ptr<SpanImpl> span(new SpanImpl(tracer));

  for (const auto& o : opts) {
    o.get().Apply(span.get());
  }

  if (span->start_timestamp_ == TimeStamp()) {
    span->start_timestamp_ = Clock::now();
  }
  if (!operation_name.empty()) {
    span->operation_name_ = operation_name;
  } else {    
    span->operation_name_ = UndefinedSpanName;
  }

  if (span->ref_.valid()) {
    // Note: Treating the ref as a ChildOf relation
    // Note: No locking required for context_ in StartSpan
    span->context_.span_id = tracer->GetOneId();
    span->context_.trace_id = span->ref_.referenced().trace_id();

    span->ref_.referenced().ForeachBaggageItem([&span](const std::string& key, const std::string& value) {
	span->context_.setBaggageItem(std::make_pair(key, value));
	return true;
      });
  } else {
    tracer->GetTwoIds(&span->context_.trace_id, &span->context_.span_id);
  }

  return span;
}

bool TracerImpl::inject(SpanContext sc, CarrierFormat format, const CarrierWriter& opaque) {
  switch (format.type()) {
  case CarrierFormat::HTTPHeaders:
  case CarrierFormat::TextMap:
    return inject_basic_carrier(sc, opaque);
  case CarrierFormat::EnvoyProto:
    return inject_envoy_carrier(sc, opaque);
  default:
    return false;
  }
}

bool TracerImpl::inject_basic_carrier(SpanContext sc, const CarrierWriter& opaque) {
  const BasicCarrierWriter* carrier = dynamic_cast<const BasicCarrierWriter*>(&opaque);
  if (carrier == nullptr) {
    return false;
  }
  carrier->Set(FieldNameTraceID, uint64ToHex(sc.trace_id()));
  carrier->Set(FieldNameSpanID, uint64ToHex(sc.span_id()));
  carrier->Set(FieldNameSampled, "true");

  sc.ForeachBaggageItem([carrier](const std::string& key,
				  const std::string& value) {
			  carrier->Set(std::string(PrefixBaggage) + key, value);
			  return true;
			});
  return true;
}

bool TracerImpl::inject_envoy_carrier(SpanContext sc, const CarrierWriter& opaque) {
  const envoy::ProtoWriter* carrier = dynamic_cast<const envoy::ProtoWriter*>(&opaque);
  if (carrier == nullptr) {
    return false;
  }
  envoy::EnvoyCarrier *const output = carrier->output_;
  output->set_span_id(sc.span_id());
  output->set_trace_id(sc.trace_id());

  sc.ForeachBaggageItem([output](const std::string& key,
				  const std::string& value) {
			  (*output->mutable_baggage_items())[key] = value;
			  return true;
			});

  envoy::LegacyProtoWriter legacy_writer(output);
  inject_basic_carrier(sc, legacy_writer);
  return true;
}

SpanContext TracerImpl::extract(CarrierFormat format, const CarrierReader& opaque) {
  switch (format.type()) {
  case CarrierFormat::HTTPHeaders:
  case CarrierFormat::TextMap:
    return extract_basic_carrier(opaque);
  case CarrierFormat::EnvoyProto:
    return extract_envoy_carrier(opaque);
  default:
    return SpanContext();
  }
}

SpanContext TracerImpl::extract_basic_carrier(const CarrierReader& opaque) {
  const BasicCarrierReader* carrier = dynamic_cast<const BasicCarrierReader*>(&opaque);
  if (carrier == nullptr) {
    return SpanContext();
  }

  std::shared_ptr<ContextImpl> ctx(new ContextImpl);
  int count = 0;
  carrier->ForeachKey([carrier, &ctx, &count](const std::string& key,
					      const std::string& value) {
			if (key == FieldNameTraceID) {
			  ctx->trace_id = util::hexToUint64(value);
			  count++;
			} else if (key == FieldNameSpanID) {
			  ctx->span_id = util::hexToUint64(value);
			  count++;
			} else if (key == FieldNameSampled) {
			  // Ignored
			  count++;
			} else if (key.size() > strlen(PrefixBaggage) &&
				   memcmp(key.data(), PrefixBaggage, strlen(PrefixBaggage)) == 0) {
			  ctx->setBaggageItem(std::make_pair(key.substr(strlen(PrefixBaggage)), value));
			}
		      });
  if (count != FieldCount) {
    return SpanContext();
  }
  return SpanContext(ctx);
}

SpanContext TracerImpl::extract_envoy_carrier(const CarrierReader& opaque) {
  const envoy::ProtoReader* carrier = dynamic_cast<const envoy::ProtoReader*>(&opaque);
  if (carrier == nullptr) {
    return SpanContext();
  }

  const envoy::EnvoyCarrier& proto = carrier->data_;
  if (proto.trace_id() == 0 && proto.span_id() == 0) {
    envoy::LegacyProtoReader legacy_reader(proto);
    return extract_basic_carrier(legacy_reader);
  }

  std::shared_ptr<ContextImpl> ctx(new ContextImpl);

  ctx->trace_id = proto.trace_id();
  ctx->span_id = proto.span_id();

  for (const auto& entry : proto.baggage_items()) {
    ctx->setBaggageItem(entry);
  }

  return SpanContext(ctx);
}

void StartTimestamp::Apply(SpanImpl *span) const {
  // Note: no locking, only called from StartSpan
  span->start_timestamp_ = when_;
}

void SetTag::Apply(SpanImpl *span) const {
  // Note: no locking, only called from StartSpan
  span->tags_.emplace(std::make_pair(key_, value_));
}

void SpanReference::Apply(SpanImpl *span) const {
  // Note: no locking, only called from StartSpan
  if (!referenced_.valid()) {
    return;
  }
  span->ref_ = *this;
}

void SpanImpl::FinishSpan(SpanFinishOptions opts) {
  for (const auto& o : opts) {
    o.get().Apply(this);
  }

  if (finish_timestamp_ == TimeStamp()) {
    finish_timestamp_ = Clock::now();
  }

  collector::Span span;
  auto tags = span.mutable_tags();

  if (ref_.valid()) {
    auto refs = span.mutable_references();
    auto ref = refs->Add();
    ref->mutable_span_context()->set_span_id(ref_.referenced().span_id());
    switch (ref_.type()) {
    case FollowsFromRef:
      ref->set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
    default:
      ref->set_relationship(collector::Reference::CHILD_OF);
      break;
    }
  }

  {
    MutexLock lock(mutex_);
    for (auto it = tags_.begin(); it != tags_.end(); ++it) {
      *tags->Add() = util::make_kv(it->first, it->second);
    }
    span.set_operation_name(operation_name_);
  }

  auto context = span.mutable_span_context();

  // Note: context_ trace_id and span_id fields do not require a lock.
  context->set_trace_id(context_.trace_id);
  context->set_span_id(context_.span_id);
  auto baggage = context->mutable_baggage();

  context_.foreachBaggageItem([baggage](const std::string& key,
					const std::string& value) {
				baggage->insert(StringMap::value_type(key, value));
				return true;
			      });

  auto duration = finish_timestamp_ - start_timestamp_;
  span.set_duration_micros(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
  *span.mutable_start_timestamp() = util::to_timestamp(start_timestamp_);

  tracer_->RecordSpan(std::move(span));
}

void FinishTimestamp::Apply(SpanImpl *span) const {
  span->finish_timestamp_ = when_;
}

void TracerImpl::RecordSpan(collector::Span&& span) {
  auto r = recorder();
  if (r) {
    r->RecordSpan(std::move(span));
  }
}

void TracerImpl::Flush() {
  auto r = recorder();
  if (r) {
    r->Flush();
  }
}

}  // namespace lightstep
