#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <memory>
#include <cstdint>
#include <mutex>
#include <vector>
using namespace opentracing;

namespace lightstep {
//------------------------------------------------------------------------------
// LightStepSpanContext
//------------------------------------------------------------------------------
namespace {
class LightStepSpanContext : public SpanContext {
 public:
  LightStepSpanContext(uint64_t trace_id, uint64_t span_id) noexcept
      : trace_id(trace_id), span_id(span_id) {}

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
// LightStepSpan
//------------------------------------------------------------------------------
namespace {
class LightStepSpan : public Span {
 public:
  LightStepSpan(std::shared_ptr<const Tracer>&& tracer,
                const StartSpanOptions& options)
      : tracer_(std::move(tracer)), span_context_(0, 0) {
    references_.resize(options.references.size());
    for (auto& reference : options.references) {
      auto span_reference_type = reference.first;
      collector::Reference collector_reference;
      switch (reference.first) {
        case SpanReferenceType::ChildOfRef:
          collector_reference.set_relationship(collector::Reference::CHILD_OF);
          break;
        case SpanReferenceType::FollowsFromRef:
          collector_reference.set_relationship(
              collector::Reference::FOLLOWS_FROM);
          break;
      }
      if (!reference.second) continue;
      auto& referenced_context =
          dynamic_cast<const LightStepSpanContext&>(*reference.second);
      collector_reference.mutable_span_context()->set_trace_id(
          referenced_context.trace_id);
      collector_reference.mutable_span_context()->set_span_id(
          referenced_context.span_id);
      references_.push_back(collector_reference);
    }
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
  std::unique_ptr<Span> StartSpanWithOptions(
      StringRef operation_name, const StartSpanOptions& options) const
      noexcept override try {
    return std::unique_ptr<Span>(
        new LightStepSpan(shared_from_this(), options));
  } catch (const std::exception&) {
    // Don't create a span if either std::bad_alloc or std::bad_cast are thrown.
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

 private:
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
