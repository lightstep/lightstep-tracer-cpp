#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <opentracing/stringref.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <tuple>
#include <vector>
#include "propagation.h"
#include "recorder.h"
#include "utility.h"
using namespace opentracing;

namespace lightstep {
std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    std::unique_ptr<Recorder>&& recorder);
//------------------------------------------------------------------------------
// to_timestamp
//------------------------------------------------------------------------------
static google::protobuf::Timestamp to_timestamp(SystemTime t) {
  using namespace std::chrono;
  auto nanos = duration_cast<nanoseconds>(t.time_since_epoch()).count();
  google::protobuf::Timestamp ts;
  const uint64_t nanosPerSec = 1000000000;
  ts.set_seconds(nanos / nanosPerSec);
  ts.set_nanos(nanos % nanosPerSec);
  return ts;
}

//------------------------------------------------------------------------------
// LightStepSpanContext
//------------------------------------------------------------------------------
namespace {
class LightStepSpanContext : public SpanContext {
 public:
  LightStepSpanContext() noexcept = default;

  LightStepSpanContext(
      uint64_t trace_id_, uint64_t span_id_,
      std::unordered_map<std::string, std::string>&& baggage) noexcept
      : trace_id(trace_id_), span_id(span_id_), baggage_(std::move(baggage)) {}

  LightStepSpanContext(const LightStepSpanContext&) = delete;
  LightStepSpanContext(LightStepSpanContext&&) = delete;

  ~LightStepSpanContext() override = default;

  LightStepSpanContext& operator=(LightStepSpanContext&) = delete;
  LightStepSpanContext& operator=(LightStepSpanContext&& other) noexcept {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    trace_id = other.trace_id;
    span_id = other.span_id;
    baggage_ = std::move(other.baggage_);
    return *this;
  }

  void setBaggageItem(string_view key, string_view value) noexcept try {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    baggage_.emplace(key, value);
  } catch (const std::bad_alloc&) {
  }

  std::string baggageItem(const std::string& key) const {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    auto lookup = baggage_.find(key);
    if (lookup != baggage_.end()) {
      return lookup->second;
    }
    return {};
  }

  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const override {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    for (const auto& bi : baggage_) {
      if (!f(bi.first, bi.second)) {
        return;
      }
    }
  }

  template <class Carrier>
  expected<void> Inject(Carrier& writer) const {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    return inject_span_context(writer, trace_id, span_id, baggage_);
  }

  template <class Carrier>
  expected<bool> Extract(Carrier& reader) {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    return extract_span_context(reader, trace_id, span_id, baggage_);
  }

  // These are modified during constructors (StartSpan and Extract),
  // but would require extra work/copying to make them const.
  uint64_t trace_id{0};
  uint64_t span_id{0};

 private:
  mutable std::mutex baggage_mutex_;
  std::unordered_map<std::string, std::string> baggage_;
};
}  // anonymous namespace

//------------------------------------------------------------------------------
// set_span_reference
//------------------------------------------------------------------------------
static bool set_span_reference(
    const std::pair<SpanReferenceType, const SpanContext*>& reference,
    std::unordered_map<std::string, std::string>& baggage,
    collector::Reference& collector_reference) {
  collector_reference.Clear();
  switch (reference.first) {
    case SpanReferenceType::ChildOfRef:
      collector_reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case SpanReferenceType::FollowsFromRef:
      collector_reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  if (reference.second == nullptr) {
    std::cerr << "LightStep: passed in null span reference.\n";
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference.second);
  if (referenced_context == nullptr) {
    std::cerr << "LightStep: passed in span reference of unexpected type.\n";
    return false;
  }
  collector_reference.mutable_span_context()->set_trace_id(
      referenced_context->trace_id);
  collector_reference.mutable_span_context()->set_span_id(
      referenced_context->span_id);

  referenced_context->ForeachBaggageItem(
      [&baggage](const std::string& key, const std::string& value) {
        baggage[key] = value;
        return true;
      });

  return true;
}

//------------------------------------------------------------------------------
// compute_start_timestamps
//------------------------------------------------------------------------------
static std::tuple<SystemTime, SteadyTime> compute_start_timestamps(
    const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respective clocks; otherwise, use the set timestamp to initialize the
  // other.
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime()) {
    return {SystemClock::now(), SteadyClock::now()};
  }
  if (start_system_timestamp == SystemTime()) {
    return {convert_time_point<SystemClock>(start_steady_timestamp),
            start_steady_timestamp};
  }
  if (start_steady_timestamp == SteadyTime()) {
    return {start_system_timestamp,
            convert_time_point<SteadyClock>(start_system_timestamp)};
  }
  return {start_system_timestamp, start_steady_timestamp};
}

//------------------------------------------------------------------------------
// LightStepSpan
//------------------------------------------------------------------------------
namespace {
class LightStepSpan : public Span {
 public:
  LightStepSpan(std::shared_ptr<const Tracer>&& tracer, Recorder& recorder,
                string_view operation_name, const StartSpanOptions& options)
      : tracer_(std::move(tracer)),
        recorder_(recorder),
        operation_name_(operation_name) {
    // Set the start timestamps.
    std::tie(start_timestamp_, start_steady_) = compute_start_timestamps(
        options.start_system_timestamp, options.start_steady_timestamp);

    // Set any span references.
    std::unordered_map<std::string, std::string> baggage;
    references_.reserve(options.references.size());
    collector::Reference collector_reference;
    for (auto& reference : options.references) {
      if (!set_span_reference(reference, baggage, collector_reference)) {
        continue;
      }
      references_.push_back(collector_reference);
    }

    // Set tags.
    for (auto& tag : options.tags) {
      tags_[tag.first] = tag.second;
    }

    // Set SpanContext.
    auto trace_id = references_.empty()
                        ? generate_id()
                        : references_[0].span_context().trace_id();
    auto span_id = generate_id();
    span_context_ = LightStepSpanContext(trace_id, span_id, std::move(baggage));
  }

  LightStepSpan(const LightStepSpan&) = delete;
  LightStepSpan(LightStepSpan&&) = delete;
  LightStepSpan& operator=(const LightStepSpan&) = delete;
  LightStepSpan& operator=(LightStepSpan&&) = delete;

  ~LightStepSpan() override {
    if (!is_finished_) {
      Finish();
    }
  }

  void FinishWithOptions(const FinishSpanOptions& options) noexcept override
      try {
    // Ensure the span is only finished once.
    if (is_finished_.exchange(true)) {
      return;
    }

    auto finish_timestamp = options.finish_steady_timestamp;
    if (finish_timestamp == SteadyTime()) {
      finish_timestamp = SteadyClock::now();
    }

    collector::Span span;

    // Set timing information.
    auto duration = finish_timestamp - start_steady_;
    span.set_duration_micros(
        std::chrono::duration_cast<std::chrono::microseconds>(duration)
            .count());
    *span.mutable_start_timestamp() = to_timestamp(start_timestamp_);

    // Set references.
    auto references = span.mutable_references();
    references->Reserve(static_cast<int>(references_.size()));
    for (const auto& reference : references_) {
      *references->Add() = reference;
    }

    // Set tags, logs, and operation name.
    {
      std::lock_guard<std::mutex> lock(mutex_);
      span.set_operation_name(std::move(operation_name_));
      auto tags = span.mutable_tags();
      tags->Reserve(static_cast<int>(tags_.size()));
      for (const auto& tag : tags_) {
        *tags->Add() = to_key_value(tag.first, tag.second);
      }
      auto logs = span.mutable_logs();
      for (auto& log : logs_) {
        *logs->Add() = log;
      }
    }

    // Set the span context.
    auto span_context = span.mutable_span_context();
    span_context->set_trace_id(span_context_.trace_id);
    span_context->set_span_id(span_context_.span_id);
    auto baggage = span_context->mutable_baggage();
    span_context_.ForeachBaggageItem(
        [baggage](const std::string& key, const std::string& value) {
          using StringMap = google::protobuf::Map<std::string, std::string>;
          baggage->insert(StringMap::value_type(key, value));
          return true;
        });

    // Record the span
    recorder_.RecordSpan(std::move(span));
  } catch (const std::bad_alloc&) {
    // Do nothing if memory allocation fails.
  }

  void SetOperationName(string_view name) noexcept override try {
    std::lock_guard<std::mutex> l(mutex_);
    operation_name_ = name;
  } catch (const std::bad_alloc&) {
    // Don't change operation name if memory can't be allocated for it.
  }

  void SetTag(string_view key, const Value& value) noexcept override try {
    std::lock_guard<std::mutex> l(mutex_);
    tags_[key] = value;
  } catch (const std::bad_alloc&) {
    // Don't add the tag if memory can't be allocated for it.
  }

  void SetBaggageItem(string_view restricted_key,
                      string_view value) noexcept override {
    span_context_.setBaggageItem(restricted_key, value);
  }

  std::string BaggageItem(string_view restricted_key) const noexcept override
      try {
    return span_context_.baggageItem(restricted_key);
  } catch (const std::bad_alloc&) {
    return {};
  }

  void Log(std::initializer_list<std::pair<string_view, Value>>
               fields) noexcept override try {
    auto timestamp = SystemClock::now();
    collector::Log log;
    *log.mutable_timestamp() = to_timestamp(timestamp);
    auto key_values = log.mutable_keyvalues();
    for (const auto& field : fields) {
      *key_values->Add() = to_key_value(field.first, field.second);
    }
    logs_.emplace_back(std::move(log));
  } catch (const std::bad_alloc&) {
    // Do nothing if memory can't be allocted for log records.
  }

  const SpanContext& context() const noexcept override { return span_context_; }
  const Tracer& tracer() const noexcept override { return *tracer_; }

 private:
  // Fields set in StartSpan() are not protected by a mutex.
  std::shared_ptr<const Tracer> tracer_;
  Recorder& recorder_;
  std::vector<collector::Reference> references_;
  SystemTime start_timestamp_;
  SteadyTime start_steady_;
  LightStepSpanContext span_context_;

  // Mutex protects tags_ and operation_name_.
  std::atomic<bool> is_finished_{false};
  std::mutex mutex_;
  std::string operation_name_;
  std::unordered_map<std::string, Value> tags_;
  std::vector<collector::Log> logs_;
};
}  // namespace

//------------------------------------------------------------------------------
// LightStepTracerImpl
//------------------------------------------------------------------------------
namespace {
class LightStepTracerImpl
    : public LightStepTracer,
      public std::enable_shared_from_this<LightStepTracerImpl> {
 public:
  explicit LightStepTracerImpl(std::unique_ptr<Recorder>&& recorder)
      : recorder_(std::move(recorder)) {}

  std::unique_ptr<Span> StartSpanWithOptions(
      string_view operation_name, const StartSpanOptions& options) const
      noexcept override try {
    return std::unique_ptr<Span>(new LightStepSpan(
        shared_from_this(), *recorder_, operation_name, options));
  } catch (const std::bad_alloc&) {
    // Don't create a span if std::bad_alloc is thrown.
    return nullptr;
  }

  expected<void> Inject(const SpanContext& sc,
                        std::ostream& writer) const override {
    return InjectImpl(sc, writer);
  }

  expected<void> Inject(const SpanContext& sc,
                        const TextMapWriter& writer) const override {
    return InjectImpl(sc, writer);
  }

  expected<void> Inject(const SpanContext& sc,
                        const HTTPHeadersWriter& writer) const override {
    return InjectImpl(sc, writer);
  }

  expected<std::unique_ptr<SpanContext>> Extract(
      std::istream& reader) const override {
    return ExtractImpl(reader);
  }

  expected<std::unique_ptr<SpanContext>> Extract(
      const TextMapReader& reader) const override {
    return ExtractImpl(reader);
  }

  expected<std::unique_ptr<SpanContext>> Extract(
      const HTTPHeadersReader& reader) const override {
    return ExtractImpl(reader);
  }

  void Close() noexcept override {
    recorder_->FlushWithTimeout(std::chrono::hours(24));
  }

 private:
  std::unique_ptr<Recorder> recorder_;

  template <class Carrier>
  expected<void> InjectImpl(const SpanContext& sc, Carrier& writer) const {
    auto lightstep_span_context =
        dynamic_cast<const LightStepSpanContext*>(&sc);
    if (lightstep_span_context == nullptr) {
      return make_unexpected(invalid_span_context_error);
    }
    return lightstep_span_context->Inject(writer);
  }

  template <class Carrier>
  expected<std::unique_ptr<SpanContext>> ExtractImpl(Carrier& reader) const {
    auto lightstep_span_context = new (std::nothrow) LightStepSpanContext();
    std::unique_ptr<SpanContext> span_context(lightstep_span_context);
    if (!span_context) {
      return make_unexpected(make_error_code(std::errc::not_enough_memory));
    }
    auto result = lightstep_span_context->Extract(reader);
    if (!result) {
      return make_unexpected(result.error());
    }
    if (!*result) {
      span_context.reset();
    }
    return span_context;
  }
};
}  // anonymous namespace

//------------------------------------------------------------------------------
// GetTraceSpanIds
//------------------------------------------------------------------------------
expected<std::array<uint64_t, 2>> LightStepTracer::GetTraceSpanIds(
    const SpanContext& sc) const noexcept {
  auto lightstep_span_context = dynamic_cast<const LightStepSpanContext*>(&sc);
  if (lightstep_span_context == nullptr) {
    return make_unexpected(invalid_span_context_error);
  }
  std::array<uint64_t, 2> result = {
      {lightstep_span_context->trace_id, lightstep_span_context->span_id}};
  return result;
}

//------------------------------------------------------------------------------
// MakeSpanContext
//------------------------------------------------------------------------------
expected<std::unique_ptr<SpanContext>> LightStepTracer::MakeSpanContext(
    uint64_t trace_id, uint64_t span_id,
    std::unordered_map<std::string, std::string>&& baggage) const noexcept try {
  std::unique_ptr<SpanContext> result(
      new LightStepSpanContext(trace_id, span_id, std::move(baggage)));
  return result;
} catch (const std::bad_alloc&) {
  return make_unexpected(make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// MakeLightStepTracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    std::unique_ptr<Recorder>&& recorder) {
  if (!recorder) {
    return MakeNoopTracer();
  }
  return std::shared_ptr<opentracing::Tracer>(
      new (std::nothrow) LightStepTracerImpl(std::move(recorder)));
}

std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    const LightStepTracerOptions& options) {
  return MakeLightStepTracer(make_lightstep_recorder(options));
}
}  // namespace lightstep
