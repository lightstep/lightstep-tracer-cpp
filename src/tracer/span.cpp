#include "tracer/span.h"

#include "tracer/serialization.h"
#include "tracer/utility.h"

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
    : serialization_chain_{new SerializationChain{}},
      stream_{serialization_chain_.get()},
      tracer_{std::move(tracer)} {
  WriteOperationName(stream_, operation_name);

  // Set the start timestamps.
  std::chrono::system_clock::time_point start_timestamp;
  std::tie(start_timestamp, start_steady_) = ComputeStartTimestamps(
      options.start_system_timestamp, options.start_steady_timestamp);
  WriteStartTimestamp(stream_, start_timestamp);

  // Set any span references.
  sampled_ = false;
  int reference_count = 0;
  for (auto& reference : options.references) {
    reference_count += static_cast<int>(SetSpanReference(reference));
  }

  // If there are any span references, sampled should be true if any of the
  // references are sampled; with no refences, we set sampled to true.
  if (reference_count == 0) {
    sampled_ = true;
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
Span::~Span() noexcept {
  if (!is_finished_) {
    Finish();
  }
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void Span::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
  (void)options;
#if 0
  // Ensure the span is only finished once.
  if (is_finished_.exchange(true)) {
    return;
  }

  std::lock_guard<std::mutex> lock_guard{mutex_};
  if (!sampled_) {
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
#endif
} catch (const std::exception& e) {
  tracer_->logger().Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void Span::SetOperationName(
    opentracing::string_view name) noexcept try {
  (void)name;
#if 0
  std::lock_guard<std::mutex> lock_guard{mutex_};
  span_.set_operation_name(name.data(), name.size());
#endif
} catch (const std::exception& e) {
  tracer_->logger().Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void Span::SetTag(opentracing::string_view key,
                           const opentracing::Value& value) noexcept try {
  (void)key;
  (void)value;
#if 0
  std::lock_guard<std::mutex> lock_guard{mutex_};
  *span_.mutable_tags()->Add() = ToKeyValue(key, value);

  if (key == opentracing::ext::sampling_priority) {
    sampled_ = is_sampled(value);
  }
#endif
} catch (const std::exception& e) {
  tracer_->logger().Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void Span::SetBaggageItem(opentracing::string_view restricted_key,
                                   opentracing::string_view value) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  baggage_.insert(std::string{restricted_key}, std::string{value});
} catch (const std::exception& e) {
  tracer_->logger().Error("SetBaggageItem failed: ", e.what());
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string Span::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto iter = baggage_.lookup(restricted_key);
  if (iter != baggage_.end()) {
    return iter->second;
  }
  return {};
} catch (const std::exception& e) {
  tracer_->logger().Error("BaggageItem failed, returning empty string: ", e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void Span::Log(std::initializer_list<
                        std::pair<opentracing::string_view, opentracing::Value>>
                            fields) noexcept try {
  (void)fields;
#if 0
  auto timestamp = SystemClock::now();
  std::lock_guard<std::mutex> lock_guard{mutex_};
  *span_.mutable_logs()->Add() =
      ToLog(timestamp, std::begin(fields), std::end(fields));
#endif
} catch (const std::exception& e) {
  tracer_->logger().Error("Log failed: ", e.what());
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void Span::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (const auto& baggage_item : baggage_) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}

//------------------------------------------------------------------------------
// sampled
//------------------------------------------------------------------------------
bool Span::sampled() const noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return sampled_;
}

//--------------------------------------------------------------------------------------------------
// SetSpanReference
//--------------------------------------------------------------------------------------------------
bool Span::SetSpanReference(
      const std::pair<opentracing::SpanReferenceType,
                      const opentracing::SpanContext*>& reference) {
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
  WriteSpanReference(stream_, reference.first, referenced_context->trace_id(),
                     referenced_context->span_id());
  sampled_ = sampled_ || referenced_context->sampled();
  referenced_context->ForeachBaggageItem(
      [this](const std::string& key, const std::string& value) {
        this->baggage_.insert(std::string{key}, std::string{value});
        return true;
      });
  return true;
}
}  // namespace lightstep
