#include "tracer/span.h"

#include <tuple>

#include "common/random.h"
#include "common/utility.h"
#include "tracer/serialization.h"
#include "tracer/tag.h"
#include "tracer/utility.h"

#include <opentracing/ext/tags.h>

using opentracing::SteadyClock;
using opentracing::SteadyTime;
using opentracing::SystemClock;
using opentracing::SystemTime;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
Span::Span(std::shared_ptr<const TracerImpl>&& tracer,
           opentracing::string_view operation_name,
           const opentracing::StartSpanOptions& options)
    : chained_stream_{new ChainedStream{}},
      header_fragment_{tracer->recorder().ReserveHeaderSpace(*chained_stream_)},
      coded_stream_{chained_stream_.get()},
      tracer_{std::move(tracer)} {
  WriteOperationName(coded_stream_, operation_name);

  // Set the start timestamps.
  std::chrono::system_clock::time_point start_timestamp;
  std::tie(start_timestamp, start_steady_) = ComputeStartTimestamps(
      tracer_->recorder(), options.start_system_timestamp,
      options.start_steady_timestamp);
  WriteStartTimestamp(coded_stream_, start_timestamp);

  // Set any span references.
  trace_flags_ = 0;
  int reference_count = 0;
  for (auto& reference : options.references) {
    uint64_t trace_id_high;
    uint64_t trace_id;
    auto valid_reference = SetSpanReference(reference, trace_id_high, trace_id);
    reference_count += static_cast<int>(valid_reference);
    if (reference_count == 1 && valid_reference) {
      trace_id_high_ = trace_id_high;
      trace_id_ = trace_id;
    }
  }

  // If there are any span references, sampled should be true if any of the
  // references are sampled; with no refences, we set sampled to true.
  if (reference_count == 0) {
    trace_flags_ = SetTraceFlag<SampledFlagMask>(trace_flags_, true);
    auto ids = GenerateIds<2>();
    trace_id_high_ = 0;
    trace_id_ = ids[0];
    span_id_ = ids[1];
  } else {
    span_id_ = GenerateId();
  }

  // Set tags.
  for (auto& tag : options.tags) {
    WriteTag(coded_stream_, tag.first, tag.second);

    // If sampling_priority is set, it overrides whatever sampling decision was
    // derived from the referenced spans.
    if (tag.first == SamplingPriorityKey) {
      trace_flags_ =
          SetTraceFlag<SampledFlagMask>(trace_flags_, is_sampled(tag.second));
    }
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
Span::~Span() noexcept {
  opentracing::FinishSpanOptions options;
  FinishImpl(options);
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void Span::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
  SpinLockGuard lock_guard{mutex_};
  FinishImpl(options);
} catch (const std::exception& e) {
  tracer_->logger().Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void Span::SetOperationName(opentracing::string_view name) noexcept try {
  SpinLockGuard lock_guard{mutex_};
  if (is_finished_) {
    return;
  }
  WriteOperationName(coded_stream_, name);
} catch (const std::exception& e) {
  tracer_->logger().Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void Span::SetTag(opentracing::string_view key,
                  const opentracing::Value& value) noexcept try {
  SpinLockGuard lock_guard{mutex_};
  if (is_finished_) {
    return;
  }
  WriteTag(coded_stream_, key, value);
  if (key == SamplingPriorityKey) {
    trace_flags_ =
        SetTraceFlag<SampledFlagMask>(trace_flags_, is_sampled(value));
  }
} catch (const std::exception& e) {
  tracer_->logger().Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void Span::SetBaggageItem(opentracing::string_view restricted_key,
                          opentracing::string_view value) noexcept try {
  auto lowercase_key = ToLower(restricted_key);
  SpinLockGuard lock_guard{mutex_};
  if (is_finished_) {
    return;
  }
  baggage_.insert_or_assign(std::move(lowercase_key), std::string{value});
} catch (const std::exception& e) {
  tracer_->logger().Error("SetBaggageItem failed: ", e.what());
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string Span::BaggageItem(opentracing::string_view restricted_key) const
    noexcept try {
  auto lowercase_key = ToLower(restricted_key);
  SpinLockGuard lock_guard{mutex_};
  auto iter = baggage_.find(lowercase_key);
  if (iter != baggage_.end()) {
    return iter->second;
  }
  return {};
} catch (const std::exception& e) {
  tracer_->logger().Error("BaggageItem failed, returning empty string: ",
                          e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void Span::Log(std::initializer_list<
               std::pair<opentracing::string_view, opentracing::Value>>
                   fields) noexcept try {
  auto timestamp = SystemClock::now();
  SpinLockGuard lock_guard{mutex_};
  if (is_finished_) {
    return;
  }
  WriteLog(coded_stream_, timestamp, fields.begin(), fields.end());
} catch (const std::exception& e) {
  tracer_->logger().Error("Log failed: ", e.what());
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void Span::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  SpinLockGuard lock_guard{mutex_};
  for (const auto& baggage_item : baggage_) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}

//------------------------------------------------------------------------------
// trace_flags
//------------------------------------------------------------------------------
uint8_t Span::trace_flags() const noexcept {
  SpinLockGuard lock_guard{mutex_};
  return trace_flags_;
}

//--------------------------------------------------------------------------------------------------
// SetSpanReference
//--------------------------------------------------------------------------------------------------
bool Span::SetSpanReference(
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>& reference,
    uint64_t& trace_id_high, uint64_t& trace_id) {
  if (reference.second == nullptr) {
    tracer_->logger().Warn("Passed in null span reference.");
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference.second);
  if (referenced_context == nullptr) {
    tracer_->logger().Warn("Passed in span reference of unexpected type.");
    return false;
  }
  trace_id_high = referenced_context->trace_id_high();
  trace_id = referenced_context->trace_id_low();
  WriteSpanReference(coded_stream_, reference.first, trace_id,
                     referenced_context->span_id());
  trace_flags_ |= referenced_context->trace_flags();
  AppendTraceState(trace_state_, referenced_context->trace_state());
  referenced_context->ForeachBaggageItem(
      [this](const std::string& key, const std::string& value) {
        this->baggage_.insert_or_assign(std::string{key}, std::string{value});
        return true;
      });
  return true;
}

//--------------------------------------------------------------------------------------------------
// FinishImpl
//--------------------------------------------------------------------------------------------------
void Span::FinishImpl(
    const opentracing::FinishSpanOptions& options) noexcept try {
  // Ensure the span is only finished once.
  if (is_finished_.exchange(true)) {
    return;
  }

  if (!IsTraceFlagSet<SampledFlagMask>(trace_flags_)) {
    return;
  }

  auto finish_timestamp = options.finish_steady_timestamp;
  if (finish_timestamp == SteadyTime()) {
    finish_timestamp = SteadyClock::now();
  }

  // Set timing information.
  auto duration = finish_timestamp - start_steady_;
  WriteDuration(coded_stream_, duration);

  // Set logs
  for (auto& log_record : options.log_records) {
    try {
      WriteLog(coded_stream_, log_record.timestamp, log_record.fields.data(),
               log_record.fields.data() + log_record.fields.size());
    } catch (const std::exception& e) {
      tracer_->logger().Error("Dropping log record: ", e.what());
    }
  }

  WriteSpanContext(coded_stream_, trace_id_, span_id_, baggage_.as_vector());

  // Record the span
  tracer_->recorder().WriteFooter(coded_stream_);
  coded_stream_.Trim();
  tracer_->recorder().RecordSpan(header_fragment_, std::move(chained_stream_));
} catch (const std::exception& e) {
  tracer_->logger().Error("FinishWithOptions failed: ", e.what());
}
}  // namespace lightstep
