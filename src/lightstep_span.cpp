#include "lightstep_span.h"
#include <opentracing/ext/tags.h>
#include <iostream>
#include "utility.h"

using opentracing::SystemTime;
using opentracing::SystemClock;
using opentracing::SteadyClock;
using opentracing::SteadyTime;

namespace lightstep {
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
      dynamic_cast<const LightStepSpanContextBase*>(reference.second);
  if (referenced_context == nullptr) {
    logger.Warn("Passed in span reference of unexpected type.");
    return false;
  }
  std::cout << "trace_id = " << referenced_context->trace_id() << std::endl;
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
    : tracer_{std::move(tracer)}, logger_{logger}, recorder_{recorder} {
  span_.set_operation_name(operation_name.data(), operation_name.size());
  auto& span_context = *span_.mutable_span_context();
  auto& baggage = *span_context.mutable_baggage();

  // Set the start timestamps.
  std::chrono::system_clock::time_point start_timestamp;
  std::tie(start_timestamp, start_steady_) = ComputeStartTimestamps(
      options.start_system_timestamp, options.start_steady_timestamp);
  *span_.mutable_start_timestamp() = ToTimestamp(start_timestamp);

  // Set any span references.
  references_.reserve(options.references.size());
  collector::Reference collector_reference;
  sampled_ = false;
  for (auto& reference : options.references) {
    if (!SetSpanReference(logger_, reference, baggage, collector_reference,
                          sampled_)) {
      continue;
    }
    references_.push_back(collector_reference);
  }

  // If there are any span references, sampled should be true if any of the
  // references are sampled; with no refences, we set sampled to true.
  if (references_.empty()) {
    sampled_ = true;
  }

  // Set tags.
  for (auto& tag : options.tags) {
    tags_[tag.first] = tag.second;
  }

  // If sampling_priority is set, it overrides whatever sampling decision was
  // derived from the referenced spans.
  auto sampling_priority_tag = tags_.find(opentracing::ext::sampling_priority);
  if (sampling_priority_tag != tags_.end()) {
    sampled_ = is_sampled(sampling_priority_tag->second);
  }

  // Set opentracing::SpanContext.
  auto trace_id = references_.empty()
                      ? GenerateId()
                      : references_[0].span_context().trace_id();
  auto span_id = GenerateId();
  span_context.set_trace_id(trace_id);
  span_context.set_span_id(span_id);
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

  // Set references.
  auto references = span_.mutable_references();
  references->Reserve(static_cast<int>(references_.size()));
  for (const auto& reference : references_) {
    *references->Add() = reference;
  }

  // Set tags, logs, and operation name.
  {
    auto tags = span_.mutable_tags();
    tags->Reserve(static_cast<int>(tags_.size()));
    for (const auto& tag : tags_) {
      try {
        *tags->Add() = ToKeyValue(tag.first, tag.second);
      } catch (const std::exception& e) {
        logger_.Error(R"(Dropping tag for key ")", tag.first,
                      R"(": )", e.what());
      }
    }
    auto logs = span_.mutable_logs();
    logs->Reserve(static_cast<int>(logs_.size()) +
                  static_cast<int>(options.log_records.size()));
    for (auto& log : logs_) {
      try {
        *logs->Add() = std::move(log);
      } catch (const std::exception& e) {
        logger_.Error("Dropping log record: ", e.what());
      }
    }
    for (auto& log_record : options.log_records) {
      try {
        collector::Log log;
        *log.mutable_timestamp() = ToTimestamp(log_record.timestamp);
        auto key_values = log.mutable_fields();
        for (auto& field : log_record.fields) {
          *key_values->Add() = ToKeyValue(field.first, field.second);
        }
        *logs->Add() = std::move(log);
      } catch (const std::exception& e) {
        logger_.Error("Dropping log record: ", e.what());
      }
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
void LightStepSpan::SetOperationName(
    opentracing::string_view name) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  span_.set_operation_name(name.data(), name.size());
} catch (const std::exception& e) {
  logger_.Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void LightStepSpan::SetTag(opentracing::string_view key,
                           const opentracing::Value& value) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  tags_[key] = value;

  if (key == opentracing::ext::sampling_priority) {
    sampled_ = is_sampled(value);
  }
} catch (const std::exception& e) {
  logger_.Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan::SetBaggageItem(opentracing::string_view restricted_key,
                                   opentracing::string_view value) noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& baggage = *span_.mutable_span_context()->mutable_baggage();
  baggage.insert(BaggageMap::value_type(restricted_key, value));
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string LightStepSpan::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& baggage = span_.span_context().baggage();
  auto lookup = baggage.find(restricted_key);
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
void LightStepSpan::Log(std::initializer_list<
                        std::pair<opentracing::string_view, opentracing::Value>>
                            fields) noexcept try {
  auto timestamp = SystemClock::now();
  collector::Log log;
  *log.mutable_timestamp() = ToTimestamp(timestamp);
  auto key_values = log.mutable_fields();
  for (const auto& field : fields) {
    *key_values->Add() = ToKeyValue(field.first, field.second);
  }
  logs_.emplace_back(std::move(log));
} catch (const std::exception& e) {
  logger_.Error("Log failed: ", e.what());
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan::ForeachBaggageItem(
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
// sampled
//------------------------------------------------------------------------------
bool LightStepSpan::sampled() const noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return sampled_;
}
}  // namespace lightstep
