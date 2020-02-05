#include "tracer/legacy/legacy_span.h"

#include "common/random.h"
#include "common/utility.h"
#include "tracer/tag.h"
#include "tracer/utility.h"

using opentracing::SteadyClock;
using opentracing::SteadyTime;
using opentracing::SystemClock;
using opentracing::SystemTime;

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LegacySpan::LegacySpan(std::shared_ptr<const opentracing::Tracer>&& tracer,
                       Logger& logger, Recorder& recorder,
                       opentracing::string_view operation_name,
                       const opentracing::StartSpanOptions& options)
    : tracer_{std::move(tracer)}, logger_{logger}, recorder_{recorder} {
  span_.set_operation_name(operation_name.data(), operation_name.size());
  auto& span_context = *span_.mutable_span_context();
  auto& baggage = *span_context.mutable_baggage();

  // Set the start timestamps.
  std::chrono::system_clock::time_point start_timestamp;
  std::tie(start_timestamp, start_steady_) =
      ComputeStartTimestamps(recorder_, options.start_system_timestamp,
                             options.start_steady_timestamp);
  *span_.mutable_start_timestamp() = ToTimestamp(start_timestamp);

  // Set any span references.
  trace_flags_ = 0;
  collector::Reference collector_reference;
  auto& references = *span_.mutable_references();
  references.Reserve(static_cast<int>(options.references.size()));
  for (auto& reference : options.references) {
    uint64_t trace_id_high;
    uint64_t trace_id;
    if (!SetSpanReference(reference, baggage, collector_reference,
                          trace_id_high, trace_id)) {
      continue;
    }
    *references.Add() = collector_reference;
    if (references.size() == 1) {
      trace_id_high_ = trace_id_high;
      span_context.set_trace_id(trace_id);
    }
  }

  // If there are any span references, sampled should be true if any of the
  // references are sampled; with no refences, we set sampled to true.
  if (references.empty()) {
    trace_flags_ = SetTraceFlag<SampledFlagMask>(trace_flags_, true);
  }

  // Set tags.
  auto& tags = *span_.mutable_tags();
  tags.Reserve(static_cast<int>(options.tags.size()));
  for (auto& tag : options.tags) {
    *tags.Add() = ToKeyValue(tag.first, tag.second);

    // If sampling_priority is set, it overrides whatever sampling decision was
    // derived from the referenced spans.
    if (tag.first == SamplingPriorityKey) {
      trace_flags_ =
          SetTraceFlag<SampledFlagMask>(trace_flags_, is_sampled(tag.second));
    }
  }

  // Set opentracing::SpanContext.
  if (references.empty()) {
    trace_id_high_ = 0;
    span_context.set_trace_id(GenerateId());
  }
  span_context.set_span_id(GenerateId());
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
LegacySpan::~LegacySpan() {
  if (!is_finished_) {
    Finish();
  }
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void LegacySpan::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
  // Ensure the span is only finished once.
  if (is_finished_.exchange(true)) {
    return;
  }

  std::lock_guard<std::mutex> lock_guard{mutex_};
  if (!IsTraceFlagSet<SampledFlagMask>(trace_flags_)) {
    return;
  }

  auto finish_timestamp = options.finish_steady_timestamp;
  if (finish_timestamp == SteadyTime()) {
    finish_timestamp = SteadyClock::now();
  }

  // Set timing information.
  auto duration = finish_timestamp - start_steady_;
  span_.set_duration_micros(
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count());

  // Set logs
  auto& logs = *span_.mutable_logs();
  logs.Reserve(logs.size() + static_cast<int>(options.log_records.size()));
  for (auto& log_record : options.log_records) {
    try {
      *logs.Add() = ToLog(log_record.timestamp, std::begin(log_record.fields),
                          std::end(log_record.fields));
    } catch (const std::exception& e) {
      logger_.Error("Dropping log record: ", e.what());
    }
  }

  // Record the span
  recorder_.RecordSpan(span_);
} catch (const std::exception& e) {
  logger_.Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void LegacySpan::SetOperationName(opentracing::string_view name) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  span_.set_operation_name(name.data(), name.size());
} catch (const std::exception& e) {
  logger_.Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void LegacySpan::SetTag(opentracing::string_view key,
                        const opentracing::Value& value) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  *span_.mutable_tags()->Add() = ToKeyValue(key, value);

  if (key == SamplingPriorityKey) {
    trace_flags_ =
        SetTraceFlag<SampledFlagMask>(trace_flags_, is_sampled(value));
  }
} catch (const std::exception& e) {
  logger_.Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void LegacySpan::SetBaggageItem(opentracing::string_view restricted_key,
                                opentracing::string_view value) noexcept try {
  auto lowercase_key = ToLower(restricted_key);
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& baggage = *span_.mutable_span_context()->mutable_baggage();
  baggage.insert(
      BaggageProtobufMap::value_type(std::move(lowercase_key), value));
} catch (const std::exception& e) {
  logger_.Error("SetBaggageItem failed: ", e.what());
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string LegacySpan::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
  auto lowercase_key = ToLower(restricted_key);
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& baggage = span_.span_context().baggage();
  auto lookup = baggage.find(lowercase_key);
  if (lookup != baggage.end()) {
    return lookup->second;
  }
  return {};
} catch (const std::exception& e) {
  logger_.Error("BaggageItem failed, returning empty string: ", e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void LegacySpan::Log(std::initializer_list<
                     std::pair<opentracing::string_view, opentracing::Value>>
                         fields) noexcept try {
  auto timestamp = SystemClock::now();
  std::lock_guard<std::mutex> lock_guard{mutex_};
  *span_.mutable_logs()->Add() =
      ToLog(timestamp, std::begin(fields), std::end(fields));
} catch (const std::exception& e) {
  logger_.Error("Log failed: ", e.what());
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LegacySpan::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (const auto& baggage_item : span_.span_context().baggage()) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}

//------------------------------------------------------------------------------
// trace_flags
//------------------------------------------------------------------------------
uint8_t LegacySpan::trace_flags() const noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return trace_flags_;
}

//--------------------------------------------------------------------------------------------------
// SetSpanReference
//--------------------------------------------------------------------------------------------------
bool LegacySpan::SetSpanReference(
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>& reference,
    BaggageProtobufMap& baggage, collector::Reference& collector_reference,
    uint64_t& trace_id_high, uint64_t& trace_id) {
  collector_reference.Clear();
  switch (reference.first) {
    case opentracing::SpanReferenceType::ChildOfRef:
      collector_reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case opentracing::SpanReferenceType::FollowsFromRef:
      collector_reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  if (reference.second == nullptr) {
    logger_.Warn("Passed in null span reference.");
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference.second);
  if (referenced_context == nullptr) {
    logger_.Warn("Passed in span reference of unexpected type.");
    return false;
  }
  trace_id_high = referenced_context->trace_id_high();
  trace_id = referenced_context->trace_id_low();
  collector_reference.mutable_span_context()->set_trace_id(trace_id);
  collector_reference.mutable_span_context()->set_span_id(
      referenced_context->span_id());

  trace_flags_ |= referenced_context->trace_flags();
  AppendTraceState(trace_state_, referenced_context->trace_state());

  referenced_context->ForeachBaggageItem(
      [&baggage](const std::string& key, const std::string& value) {
        baggage[key] = value;
        return true;
      });

  return true;
}
}  // namespace lightstep
