#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <memory>
#include <cstdint>
#include <mutex>
#include <vector>
#include <iostream>
#include <tuple>
#include <random>
using namespace opentracing;

namespace lightstep {
//------------------------------------------------------------------------------
// LightStepSpanContext
//------------------------------------------------------------------------------
namespace {
class LightStepSpanContext : public SpanContext {
 public:
  LightStepSpanContext() noexcept : trace_id(0), span_id(0) {}

  LightStepSpanContext(
      uint64_t trace_id, uint64_t span_id,
      std::unordered_map<std::string, std::string>&& baggage) noexcept
      : trace_id(trace_id), span_id(span_id), baggage_(std::move(baggage)) {}

  void setBaggageItem(StringRef key, StringRef value) noexcept try {
    std::lock_guard<std::mutex> l(baggage_mutex_);
    baggage_.emplace(key, value);
  } catch(const std::bad_alloc&) {
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

  // These are modified during constructors (StartSpan and Extract),
  // but would require extra work/copying to make them const.
  uint64_t trace_id;
  uint64_t span_id;

private:
  mutable std::mutex baggage_mutex_;
  std::unordered_map<std::string, std::string> baggage_;
};
} // anonymous namespace

//------------------------------------------------------------------------------
// set_span_reference
//------------------------------------------------------------------------------
static bool set_span_reference(
    const std::pair<SpanReferenceType, const SpanContext*>& reference,
    std::unordered_map<std::string, std::string>& baggage,
    collector::Reference& collector_reference) {
  switch (reference.first) {
    case SpanReferenceType::ChildOfRef:
      collector_reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case SpanReferenceType::FollowsFromRef:
      collector_reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  if (!reference.second) {
    std::cerr
        << "LightStep: passed in null span reference.\n";
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference.second);
  if (!referenced_context) {
    std::cerr
        << "LightStep: passed in span reference of unexpected type.\n";
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
std::tuple<SystemTime, SteadyTime> compute_start_timestamps(
    const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respetive clocks; otherwise, use the set timestamp to initialize the other
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime())
    return {SystemClock::now(), SteadyClock::now()};
  else if (start_system_timestamp == SystemTime())
    return {SystemTime(std::chrono::duration_cast<SystemClock::duration>(
                start_steady_timestamp.time_since_epoch())),
            start_steady_timestamp};
  else
    return {start_system_timestamp,
            SteadyTime(std::chrono::duration_cast<SteadyClock::duration>(
                start_system_timestamp.time_since_epoch()))};
}

//------------------------------------------------------------------------------
// LightStepSpan
//------------------------------------------------------------------------------
namespace {
class LightStepSpan : public Span {
 public:
  LightStepSpan(std::shared_ptr<const Tracer>&& tracer,
                const StartSpanOptions& options)
      : tracer_(std::move(tracer)) {
    // Set the start timestamps.
    std::tie(start_timestamp_, start_steady_) = compute_start_timestamps(
        options.start_system_timestamp, options.start_steady_timestamp);

    // Set any span references.
    std::unordered_map<std::string, std::string> baggage;
    references_.reserve(options.references.size());
    collector::Reference collector_reference;
    for (auto& reference : options.references) {
      auto span_reference_type = reference.first;
      if (!set_span_reference(reference, baggage, collector_reference))
        continue;
      references_.push_back(collector_reference);
    }


    // Set tags.
    for (auto& tag : options.tags)
      tags_[tag.first] = tag.second;

    // Set SpanContext.

  }

  void FinishWithOptions(
      const FinishSpanOptions& options) noexcept override {
    auto finish_timestamp = options.finish_steady_timestamp;
    if (finish_timestamp == SteadyTime())
      finish_timestamp = SteadyClock::now();

    collector::Span span;
    auto tags = span.mutable_tags();
  }

  void SetOperationName(StringRef name) noexcept override try {
    std::lock_guard<std::mutex> l(mutex_);
    operation_name_ = name;
  } catch (const std::bad_alloc&) {
    // Don't change operation name if memory can't be allocated for it.
  }

  void SetTag(StringRef key, const Value& value) noexcept override try {
    std::lock_guard<std::mutex> l(mutex_);
    tags_[key] = value;
  } catch (const std::bad_alloc&) {
    // Don't add the tag if memory can't be allocated for it.
  }

  void SetBaggageItem(StringRef restricted_key,
                      StringRef value) noexcept override {
    span_context_.setBaggageItem(restricted_key, value);
  }

  std::string BaggageItem(StringRef restricted_key) const noexcept override
      try {
    return span_context_.baggageItem(restricted_key);
  } catch (const std::bad_alloc&) {
    return {};
  }

  const SpanContext& context() const noexcept override { return span_context_; }
  const Tracer& tracer() const noexcept override { return *tracer_; }

 private:
  // Fields set in StartSpan() are not protected by a mutex.
  std::shared_ptr<const Tracer> tracer_;
  std::vector<collector::Reference> references_;
  SystemTime    start_timestamp_;
  SteadyTime    start_steady_;
  LightStepSpanContext span_context_;

  // Mutex protects tags_ and operation_name_.
  std::mutex    mutex_;
  std::string   operation_name_;
  std::unordered_map<std::string, Value> tags_;
};
} // namespace anonymous

//------------------------------------------------------------------------------
// LightStepTracer
//------------------------------------------------------------------------------
namespace {
class LightStepTracer : public Tracer,
                        public std::enable_shared_from_this<LightStepTracer> {
 public:
  LightStepTracer() : rand_source_(std::random_device()()) {}

  std::unique_ptr<Span> StartSpanWithOptions(
      StringRef operation_name, const StartSpanOptions& options) const
      noexcept override try {
    return std::unique_ptr<Span>(
        new LightStepSpan(shared_from_this(), options));
  } catch (const std::bad_alloc&) {
    // Don't create a span if std::bad_alloc is thrown.
    return nullptr;
  }

  Expected<void, std::string> Inject(
      const SpanContext& sc, CarrierFormat format,
      const CarrierWriter& writer) const override {
    return {};
  }

  std::unique_ptr<SpanContext> Extract(
      CarrierFormat format, const CarrierReader& reader) const override {
    return nullptr;
  }

  void GetTwoIds(uint64_t* a, uint64_t* b) {
    std::lock_guard<std::mutex> l(mutex_);
    *a = rand_source_();
    *b = rand_source_();
  }

  uint64_t GetOneId() {
    std::lock_guard<std::mutex> l(mutex_);
    return rand_source_();
  }

 private:
  // Protects rand_source_, recorder_.
  std::mutex mutex_;

  // N.B. This may become a source of contention, if and when it does,
  // it may be converted to use a thread-local cache or thread-local
  // source.
  std::mt19937_64 rand_source_;
};
} // anonymous namespace

//------------------------------------------------------------------------------
// make_lightstep_tracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> make_lightstep_tracer() noexcept {
  return std::shared_ptr<opentracing::Tracer>(new (std::nothrow)
                                                  LightStepTracer());
}
} // namespace lightstep
