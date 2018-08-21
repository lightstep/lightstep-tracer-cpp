#include "lightstep_span.h"
#include <opentracing/ext/tags.h>
#include "utility.h"

using opentracing::SystemTime;
using opentracing::SystemClock;
using opentracing::SteadyClock;
using opentracing::SteadyTime;

namespace lightstep {
//------------------------------------------------------------------------------
// ToLog
//------------------------------------------------------------------------------
template <class Iterator>
static std::unique_ptr<collector::Log> ToLog(
    std::chrono::system_clock::time_point timestamp, Iterator field_first,
    Iterator field_last) {
  std::unique_ptr<collector::Log> result{new collector::Log{}};
  *result->mutable_timestamp() = ToTimestamp(timestamp);
  auto key_values = result->mutable_fields();
  for (Iterator field_iter = field_first; field_iter != field_last;
       ++field_iter) {
    std::unique_ptr<collector::KeyValue> key_value{new collector::KeyValue{}};
    ToKeyValue(field_iter->first, field_iter->second, *key_value);
    key_values->AddAllocated(key_value.release());
  }
  return result;
}

//------------------------------------------------------------------------------
// is_sampled
//------------------------------------------------------------------------------
static bool is_sampled(const opentracing::Value& value) {
  return value != opentracing::Value{0} && value != opentracing::Value{0u};
}

//------------------------------------------------------------------------------
// ComputeStartTimestamps
//------------------------------------------------------------------------------
static std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
    const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respective clocks; otherwise, use the set timestamp to initialize the
  // other.
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{SystemClock::now(),
                                              SteadyClock::now()};
  }
  if (start_system_timestamp == SystemTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        opentracing::convert_time_point<SystemClock>(start_steady_timestamp),
        start_steady_timestamp};
  }
  if (start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        start_system_timestamp,
        opentracing::convert_time_point<SteadyClock>(start_system_timestamp)};
  }
  return std::tuple<SystemTime, SteadyTime>{start_system_timestamp,
                                            start_steady_timestamp};
}

//------------------------------------------------------------------------------
// SetSpanReference
//------------------------------------------------------------------------------
static bool SetSpanReference(
    Logger& logger,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>& reference,
    BaggageMap& baggage, collector::Reference& collector_reference,
    bool& sampled) {
  switch (reference.first) {
    case opentracing::SpanReferenceType::ChildOfRef:
      collector_reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case opentracing::SpanReferenceType::FollowsFromRef:
      collector_reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  if (reference.second == nullptr) {
    logger.Warn("Passed in null span reference.");
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference.second);
  if (referenced_context == nullptr) {
    logger.Warn("Passed in span reference of unexpected type.");
    return false;
  }
  collector_reference.mutable_span_context()->set_trace_id(
      referenced_context->trace_id());
  collector_reference.mutable_span_context()->set_span_id(
      referenced_context->span_id());
  sampled = sampled || referenced_context->sampled();

  referenced_context->ForeachBaggageItem(
      [&baggage](const std::string& key, const std::string& value) {
        baggage[key] = value;
        return true;
      });

  return true;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepSpan::LightStepSpan(
    std::shared_ptr<const opentracing::Tracer>&& tracer, Logger& logger,
    Recorder& recorder, opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options)
    : tracer_{std::move(tracer)},
      logger_{logger},
      recorder_{recorder},
      data_{new collector::Span} {
  // Set operation name
  data_->set_operation_name(operation_name.data(), operation_name.size());

  // Set the start timestamps.
  std::tie(start_timestamp_, start_steady_) = ComputeStartTimestamps(
      options.start_system_timestamp, options.start_steady_timestamp);

  // Set any span references.
  BaggageMap baggage;
  auto& references = *data_->mutable_references();
  references.Reserve(static_cast<int>(options.references.size()));
  bool sampled = false;
  for (auto& reference : options.references) {
    std::unique_ptr<collector::Reference> collector_reference{
        new collector::Reference{}};
    if (!SetSpanReference(logger_, reference, baggage, *collector_reference,
                          sampled)) {
      continue;
    }
    references.AddAllocated(collector_reference.release());
  }

  // If there are any span references, sampled should be true if any of the
  // references are sampled; with no refences, we set sampled to true.
  if (references.empty()) {
    sampled = true;
  }

  // Set tags.
  auto& tags = *data_->mutable_tags();
  tags.Reserve(static_cast<int>(options.tags.size()));
  for (auto& tag : options.tags) {
    std::unique_ptr<collector::KeyValue> key_value{new collector::KeyValue{}};
    ToKeyValue(tag.first, tag.second, *key_value);
    tags.AddAllocated(key_value.release());
    // If sampling_priority is set, it overrides whatever sampling decision was
    // derived from the referenced spans.
    if (tag.first == opentracing::ext::sampling_priority) {
      sampled = is_sampled(tag.second);
    }
  }

  // Set opentracing::SpanContext.
  auto trace_id = references.empty() ? GenerateId()
                                     : references[0].span_context().trace_id();
  auto span_id = GenerateId();
  span_context_ =
      LightStepSpanContext{trace_id, span_id, sampled, std::move(baggage)};
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
LightStepSpan::~LightStepSpan() {
  if (!is_finished_) {
    Finish();
  }
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void LightStepSpan::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
  // Ensure the span is only finished once.
  if (is_finished_.exchange(true)) {
    return;
  }

  // If the span isn't sampled do nothing.
  if (!span_context_.sampled()) {
    return;
  }

  auto finish_timestamp = options.finish_steady_timestamp;
  if (finish_timestamp == SteadyTime()) {
    finish_timestamp = SteadyClock::now();
  }

  // Set timing information.
  auto duration = finish_timestamp - start_steady_;
  data_->set_duration_micros(
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
  *data_->mutable_start_timestamp() = ToTimestamp(start_timestamp_);

  // Set Logs
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& logs = *data_->mutable_logs();
  logs.Reserve(logs.size() + static_cast<int>(options.log_records.size()));
  for (auto& log_record : options.log_records) {
    try {
      auto log = ToLog(log_record.timestamp, std::begin(log_record.fields),
                       std::end(log_record.fields));
      logs.AddAllocated(log.release());
    } catch (const std::exception& e) {
      logger_.Error("Dropping log record: ", e.what());
    }
  }

  // Set the span context.
  auto span_context = data_->mutable_span_context();
  span_context->set_trace_id(span_context_.trace_id());
  span_context->set_span_id(span_context_.span_id());
  auto baggage = span_context->mutable_baggage();
  span_context_.ForeachBaggageItem(
      [baggage](const std::string& key, const std::string& value) {
        using StringMap = google::protobuf::Map<std::string, std::string>;
        baggage->insert(StringMap::value_type(key, value));
        return true;
      });

  // Record the span
  recorder_.RecordSpan(std::move(data_));
} catch (const std::exception& e) {
  logger_.Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void LightStepSpan::SetOperationName(
    opentracing::string_view name) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  if (data_ == nullptr) {
    return;
  }
  data_->set_operation_name(name.data(), name.size());
} catch (const std::exception& e) {
  logger_.Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void LightStepSpan::SetTag(opentracing::string_view key,
                           const opentracing::Value& value) noexcept try {
  {
    std::unique_ptr<collector::KeyValue> key_value{new collector::KeyValue{}};
    ToKeyValue(key, value, *key_value);
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (data_ == nullptr) {
      return;
    }
    data_->mutable_tags()->AddAllocated(key_value.release());
  }

  if (key == opentracing::ext::sampling_priority) {
    span_context_.set_sampled(is_sampled(value));
  }
} catch (const std::exception& e) {
  logger_.Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan::SetBaggageItem(opentracing::string_view restricted_key,
                                   opentracing::string_view value) noexcept {
  span_context_.set_baggage_item(restricted_key, value);
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string LightStepSpan::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
  return span_context_.baggage_item(restricted_key);
} catch (const std::exception& e) {
  logger_.Error("BaggageItem failed, returning empty string: ", e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void LightStepSpan::Log(std::initializer_list<
                        std::pair<opentracing::string_view, opentracing::Value>>
                            fields) noexcept try {
  auto timestamp = SystemClock::now();
  auto log = ToLog(timestamp, std::begin(fields), std::end(fields));
  std::lock_guard<std::mutex> lock_guard{mutex_};
  if (data_ == nullptr) {
    return;
  }
  data_->mutable_logs()->AddAllocated(log.release());
} catch (const std::exception& e) {
  logger_.Error("Log failed: ", e.what());
}
}  // namespace lightstep
