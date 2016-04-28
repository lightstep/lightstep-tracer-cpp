#include <random>

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "lightstep_thrift/lightstep_types.h"

#include "impl.h"
#include "options.h"
#include "recorder.h"

namespace lightstep {
namespace {

using namespace lightstep_thrift;

const char TraceKeyPrefix[] = "join:";
const char TraceGUIDKey[] = "join:trace_guid";
const char ParentSpanGUIDKey[] = "parent_span_guid";
const char UndefinedSpanName[] = "undefined";

KeyValue make_kv(const std::string& key, const std::string& value) {
  KeyValue kv;
  kv.__set_Key(key);
  kv.__set_Value(value);
  return kv;
}

TraceJoinId make_join(const std::string& key, const std::string& value) {
  TraceJoinId kv;
  kv.__set_TraceKey(key);
  kv.__set_Value(value);
  return kv;
}

} // namespace

TracerImpl::TracerImpl(const TracerOptions& options)
  : options_(options),
    rand_source_(std::random_device()()),
    runtime_guid_(util::id_to_string(GetOneId())),
    runtime_micros_(util::to_micros(Clock::now())) {
  component_name_ = options.component_name.empty() ? util::program_name() : options.component_name;
  for (auto it = options.tags.begin();
       it != options.tags.end(); ++it) {
    runtime_attributes_.emplace_back(make_kv(it->first, it->second));
  }
  runtime_attributes_.emplace_back(make_kv("lightstep_tracer_platform", "c++"));
  runtime_attributes_.emplace_back(make_kv("lightstep_tracer_version", "0.8"));
  if (!options.recorder) {
    abort();
  }
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

std::unique_ptr<SpanImpl> TracerImpl::StartSpan(std::shared_ptr<TracerImpl> selfptr,
						const StartSpanOptions& options) {
  std::unique_ptr<SpanImpl> span(new SpanImpl);

  TimeStamp start_time = options.start_time == TimeStamp() ? Clock::now() : options.start_time;

  span->tracer_         = selfptr;
  span->operation_name_ = options.operation_name.empty() ? UndefinedSpanName : options.operation_name;
  span->start_micros_   = util::to_micros(start_time);
  span->tags_           = options.tags;

  GetTwoIds(&span->context_.trace_id, &span->context_.span_id);

  auto parent = options.parent.impl();
  if (parent == nullptr) {
    span->context_.sampled = options_.should_sample(span->context_.trace_id);
  } else {
    MutexLock l(parent->mutex_);

    // Note: Should also check (assert?) the Tracers are the same.
    span->context_.parent_span_id = parent->context_.span_id;
    span->context_.baggage = parent->context_.baggage;
    span->context_.sampled = parent->context_.sampled;
  }

  return span;
}

void SpanImpl::FinishSpan(const FinishSpanOptions& options) {
  TimeStamp finish_time = (options.finish_time == TimeStamp() ?
			   Clock::now() :
			   options.finish_time);

  using namespace apache::thrift::transport;
  using namespace apache::thrift::protocol;

  boost::shared_ptr<TMemoryBuffer> memory(new TMemoryBuffer(4096));
  TBinaryProtocol proto(memory);

  SpanRecord span;
  MutexLock l(mutex_);

  std::vector<KeyValue> attrs{
    make_kv(ParentSpanGUIDKey, util::id_to_string(context_.parent_span_id)),
  };
  std::vector<TraceJoinId> joins{
    make_join(TraceGUIDKey, util::id_to_string(context_.trace_id)),
  };

  for (auto it = tags_.begin(); it != tags_.end(); ++it) {
    const std::string& key = it->first;
    if (key.compare(0, strlen(TraceKeyPrefix), TraceKeyPrefix) == 0) {
      joins.emplace_back(make_join(key, it->second.to_string()));
    } else {
      attrs.emplace_back(make_kv(key, it->second.to_string()));
    }
  }

  span.__set_span_guid(util::id_to_string(context_.span_id));
  span.__set_runtime_guid(tracer_->runtime_guid_);
  span.__set_span_name(operation_name_);
  span.__set_oldest_micros(start_micros_);
  span.__set_youngest_micros(util::to_micros(finish_time));

  span.__set_join_ids(std::move(joins));
  span.__set_attributes(std::move(attrs));

  // TODO! Note that thrift needs its c++v2 generator or a special
  // compiler option (not present in 0.9.2) to generate move
  // constructors, so this is not efficient.  Must be addressed.
  tracer_->options_.recorder->ReportSpan(std::move(span));
}

}  // namespace lightstep
