#include <random>

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "lightstep_thrift/lightstep_types.h"

#include "impl.h"
#include "options.h"

namespace lightstep {
namespace {

const char UndefinedSpanName[] = "undefined";

} // namespace

TracerImpl::TracerImpl(const TracerOptions& options)
  : options_(options),
    rand_source_(std::random_device()()) {}

void TracerImpl::GetTwoIds(uint64_t *a, uint64_t *b) {
  MutexLock l(mutex_);
  *a = rand_source_();  
  *b = rand_source_();
}

std::unique_ptr<SpanImpl> TracerImpl::StartSpan(std::shared_ptr<TracerImpl> selfptr,
						const StartSpanOptions& options) {
  std::unique_ptr<SpanImpl> span(new SpanImpl);

  span->tracer_         = selfptr;
  span->operation_name_ = options.operation_name.empty() ? UndefinedSpanName : options.operation_name;
  span->start_time_     = options.start_time == TimeStamp() ? Clock::now() : options.start_time;
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
  MutexLock l(mutex_);

  using namespace apache::thrift::transport;
  using namespace apache::thrift::protocol;

  boost::shared_ptr<TMemoryBuffer> memory(new TMemoryBuffer(4096));
  TBinaryProtocol proto(memory);

  lightstep_thrift::Runtime runtime;
  lightstep_thrift::SpanRecord sr;
  lightstep_thrift::ReportRequest rr;
  runtime.guid = "test_guid";
  sr.__set_span_name(operation_name_);
  rr.__set_runtime(std::move(runtime));
  rr.__set_span_records({std::move(sr)});
  int n = rr.write(&proto);

  uint8_t *buffer;
  uint32_t length;
  memory->getBuffer(&buffer, &length);
  tracer_->options_.binary_transport(buffer, length);
  if (length != n) {
    abort();
  }
}

}  // namespace lightstep
