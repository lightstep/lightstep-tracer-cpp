#include <string.h>
#include <functional>
#include <random>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "lightstep/impl.h"
#include "lightstep/options.h"
#include "lightstep/tracer.h"
#include "lightstep/util.h"

// NOTE that protobuf doesn't have C++11 move constructors. The code
// below does somewhat more allocation and copying than neccesary as a
// result.

namespace lightstep {

const char TracerIDKey[] = "lightstep.guid";

namespace {

const char TraceKeyPrefix[] = "join:";
const char ParentSpanGUIDKey[] = "parent_span_guid";
const char UndefinedSpanName[] = "undefined";
const char ComponentNameKey[] = "lightstep.component_name";
const char PlatformNameKey[] = "lightstep.tracer_platform";
const char VersionNameKey[] = "lightstep.tracer_version";

#define PREFIX_TRACER_STATE "ot-tracer-"
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
  ss << u;
  return ss.str();
}

} // namespace

TracerImpl::TracerImpl(const TracerOptions& options_in)
  : options_(options_in),
    rand_source_(std::random_device()()),
    tracer_start_time_(Clock::now()) {

  tracer_id_ = GetOneId();

  if (options_.tracer_attributes.find(ComponentNameKey) == options_.tracer_attributes.end()) {
    options_.tracer_attributes.emplace(std::make_pair(ComponentNameKey, util::program_name()));
  }
  // Note: drop the TracerIDKey, this is copied from tracer_id_.
  auto guid_lookup = options_.tracer_attributes.find(TracerIDKey);
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

bool TracerImpl::inject(SpanContext sc, const CarrierFormat& format, const CarrierWriter& opaque) {
  const BuiltinCarrierFormat *bcf = dynamic_cast<const BuiltinCarrierFormat*>(&format);
  if (bcf == nullptr) {
    return false;
  }
  switch (bcf->type()) {
  case BuiltinCarrierFormat::HTTPHeaders:
  case BuiltinCarrierFormat::TextMap:
    break;
  default:
    return false;
  }

  const TextMapWriter* carrier = dynamic_cast<const TextMapWriter*>(&opaque);
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

SpanContext TracerImpl::extract(const CarrierFormat& format, const CarrierReader& opaque) {
  const BuiltinCarrierFormat *bcf = dynamic_cast<const BuiltinCarrierFormat*>(&format);
  // TODO a way to log errors here
  if (bcf == nullptr) {
    return SpanContext();
  }
  switch (bcf->type()) {
  case BuiltinCarrierFormat::HTTPHeaders:
  case BuiltinCarrierFormat::TextMap:
    break;
  default:
    return SpanContext();
  }

  const TextMapReader* carrier = dynamic_cast<const TextMapReader*>(&opaque);
  if (carrier == nullptr) {
    return SpanContext();
  }

  std::shared_ptr<ContextImpl> ctx(new ContextImpl);
  int count = 0;
  carrier->ForeachKey([carrier, &ctx, &count](const std::string& key,
					      const std::string& value) {
			if (key == FieldNameTraceID) {
			  ctx->trace_id = util::stringToUint64(value);
			  count++;
			} else if (key == FieldNameSpanID) {
			  ctx->span_id = util::stringToUint64(value);
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
    *tags->Add() = util::make_kv(ParentSpanGUIDKey, ref_.referenced().span_id());
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
