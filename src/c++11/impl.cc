#include <string.h>
#include <functional>
#include <random>
#include <sstream>

#include "config.h"
#include "impl.h"
#include "options.h"
#include "tracer.h"
#include "types.h"
#include "util.h"

namespace lightstep {
namespace {

using namespace lightstep_net;

const char TraceKeyPrefix[] = "join:";
const char ParentSpanGUIDKey[] = "parent_span_guid";
const char UndefinedSpanName[] = "undefined";
const char ComponentNameKey[] = "lightstep.component_name";
const char PlatformNameKey[] = "lightstep.tracer_platform";
const char VersionNameKey[] = "lightstep.tracer_version";

#define PREFIX_TRACER_STATE "ot-tracer-"
const char PrefixTracerState[] = PREFIX_TRACER_STATE;
const char PrefixBaggage[] = "ot-baggage-";

const char FieldNameTraceID[] = PREFIX_TRACER_STATE "traceid";
const char FieldNameSpanID[] = PREFIX_TRACER_STATE "spanid";
const char FieldNameSampled[] = PREFIX_TRACER_STATE "sampled";

std::string uint64ToHex(uint64_t u) {
  std::stringstream ss;
  ss << u;
  return ss.str();
}

uint64_t stringToUint64(const std::string& s) {
  // TODO error handling
  std::stringstream ss(s);
  uint64_t x;
  ss >> x;
  return x;
}

} // namespace

TracerImpl::TracerImpl(const TracerOptions& options_in)
  : options_(options_in),
    rand_source_(std::random_device()()),
    runtime_guid_(util::id_to_string(GetOneId())),
    runtime_micros_(util::to_micros(Clock::now())) {

  if (options_.runtime_attributes.find(ComponentNameKey) == options_.runtime_attributes.end()) {
    options_.runtime_attributes.emplace(std::make_pair(ComponentNameKey, util::program_name()));
  }

  options_.runtime_attributes.emplace(std::make_pair(PlatformNameKey, "C++11"));
  options_.runtime_attributes.emplace(std::make_pair(VersionNameKey, PACKAGE_VERSION));
}

void TracerImpl::GetTwoIds(uint64_t *a, uint64_t *b) {
  MutexLock l(mutex_);
  *a = rand_source_();  
  *b = rand_source_();
}

uint64_t TracerImpl::GetOneId() {
  MutexLock l(mutex_);
  return rand_source_();
}

void TracerImpl::set_recorder(std::unique_ptr<Recorder> recorder) {
  recorder_ = std::shared_ptr<Recorder>(std::move(recorder));
}

std::unique_ptr<SpanImpl> TracerImpl::StartSpan(std::shared_ptr<TracerImpl> selfptr,
						const std::string& operation_name,
						SpanStartOptions opts) {
  std::unique_ptr<SpanImpl> span(new SpanImpl(selfptr));

  for (const auto& o : opts) {
    o.get().Apply(span.get());
  }

  if (span->start_micros_ == 0) {
    span->start_micros_ = util::to_micros(Clock::now());
  }
  span->operation_name_ = operation_name;
  if (span->operation_name_.empty()) {
    span->operation_name_ = UndefinedSpanName;
  }

  if (span->context_.trace_id == 0) {
    GetTwoIds(&span->context_.trace_id, &span->context_.span_id);

    span->context_.sampled = (options_.should_sample == nullptr ?
   			      true :
   			      options_.should_sample(span->context_.trace_id));
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
  carrier->Set(FieldNameSampled, sc.sampled() ? "true" : "false");

  sc.ForeachBaggageItem([carrier](const std::string& key,
				  const std::string& value) {
			  carrier->Set(std::string(PrefixBaggage) + key, value);
			  return false;
			});
  return true;
}

SpanContext TracerImpl::extract(const CarrierFormat& format, const CarrierReader& opaque) {
  const BuiltinCarrierFormat *bcf = dynamic_cast<const BuiltinCarrierFormat*>(&format);
  if (bcf == nullptr) {
    // TODO error handling
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
  carrier->ForeachKey([carrier, &ctx](const std::string& key,
				const std::string& value) {
			if (key == FieldNameTraceID) {
			  ctx->trace_id = stringToUint64(value);
			} else if (key == FieldNameSpanID) {
			  ctx->span_id = stringToUint64(value);
			} else if (key == FieldNameSampled) {
			  ctx->sampled = (value == "true");
			} else if (key.size() > strlen(PrefixBaggage) &&
				   memcmp(key.data(), PrefixBaggage, strlen(PrefixBaggage)) == 0) {
			  ctx->setBaggageItem(std::make_pair(key.substr(strlen(PrefixBaggage)), value));
			}
		      });
  return SpanContext(ctx);
}

void StartTimestamp::Apply(SpanImpl *span) const {
  span->start_micros_ = util::to_micros(when_);
}

void AddTag::Apply(SpanImpl *span) const {
  span->tags_.emplace(std::make_pair(key_, value_));
}

void SpanReference::Apply(SpanImpl *span) const {
  span->context_.span_id = span->tracer_->GetOneId();
  span->context_.trace_id = referenced_.trace_id();
  span->context_.parent_span_id = referenced_.span_id();

  referenced_.ForeachBaggageItem([span](const std::string& key, const std::string& value) {
      span->context_.setBaggageItem(std::make_pair(key, value));
      return true;
    });
  
  span->context_.sampled = referenced_.sampled();
}

void SpanImpl::FinishSpan(SpanFinishOptions opts) {
  SpanRecord span;

  for (const auto& o : opts) {
    o.get().Apply(&span);
  }

  if (span.youngest_micros == 0) {
    span.youngest_micros = util::to_micros(Clock::now());
  }

  MutexLock l(mutex_);

  std::vector<KeyValue> attrs{
    util::make_kv(ParentSpanGUIDKey, util::id_to_string(context_.parent_span_id)),
  };
  std::vector<TraceJoinId> joins;

  for (auto it = tags_.begin(); it != tags_.end(); ++it) {
    const std::string& key = it->first;
    if (key.compare(0, strlen(TraceKeyPrefix), TraceKeyPrefix) == 0) {
      joins.emplace_back(util::make_join(key, it->second.to_string()));
    } else {
      attrs.emplace_back(util::make_kv(key, it->second.to_string()));
    }
  }

  span.trace_guid = util::id_to_string(context_.trace_id);
  span.span_guid = util::id_to_string(context_.span_id);
  span.runtime_guid = tracer_->runtime_guid();
  span.span_name = operation_name_;
  span.oldest_micros = start_micros_;

  span.join_ids = std::move(joins);
  span.attributes = std::move(attrs);

  tracer_->RecordSpan(std::move(span));
}

void FinishTimestamp::Apply(lightstep_net::SpanRecord *span) const {
  span->youngest_micros = util::to_micros(when_);
}

void TracerImpl::RecordSpan(lightstep_net::SpanRecord&& span) {
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
