#include <random>

#include "impl.h"
#include "options.h"

namespace lightstep {
namespace {

const char UnnamedSpanName[] = "omitted";

bool ShouldSample() {
  // @@@ Placeholder for TracerOptions
  return false;
}

} // namespace

TracerImpl::TracerImpl(const TracerOptions& options)
  : options_(options),
    rand_source_(std::random_device()()) {}

void TracerImpl::GetTwoIds(uint64_t *a, uint64_t *b) {
  MutexLock l(mutex_);
  *a = rand_source_();  
  *b = rand_source_();
}

std::unique_ptr<SpanImpl> TracerImpl::StartSpan(std::shared_ptr<TracerImpl> tracer,
						const StartSpanOptions& options) {
  std::unique_ptr<SpanImpl> span(new SpanImpl);

  span->tracer_ = tracer;
  span->operation_name_ = (options.operation_name.empty() ?
			   UnnamedSpanName :
			   options.operation_name);
  span->start_time_ = (options.start_time == TimeStamp() ?
		      Clock::now() :
		      options.start_time);
  span->tags_ = options.tags;

  GetTwoIds(&span->context_.trace_id, &span->context_.span_id);

  auto parent = options.parent.impl();
  if (parent == nullptr) {
    span->context_.sampled = ShouldSample();
  } else {
    MutexLock l(parent->mutex_);

    // Note: Should also check (assert?) the Tracers are the same.
    span->context_.parent_span_id = parent->context_.span_id;
    span->context_.baggage = parent->context_.baggage;
    span->context_.sampled = parent->context_.sampled;
  }

  return span;
}

}  // namespace lightstep
