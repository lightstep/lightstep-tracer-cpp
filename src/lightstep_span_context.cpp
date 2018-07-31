#include "lightstep_span_context.h"

using StringMap = google::protobuf::Map<std::string, std::string>;

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepSpanContext::LightStepSpanContext(
    uint64_t trace_id, uint64_t span_id,
    std::unordered_map<std::string, std::string>&& baggage) noexcept
    : LightStepSpanContext{trace_id, span_id, true, std::move(baggage)} {}

LightStepSpanContext::LightStepSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    std::unordered_map<std::string, std::string>&& baggage) noexcept
    : sampled_{sampled} {
  data_.set_trace_id(trace_id);
  data_.set_span_id(span_id);
  auto& baggage_data = *data_.mutable_baggage();
  for (auto& baggage_item : baggage)
    baggage_data.insert(
        StringMap::value_type(baggage_item.first, baggage_item.second));
}

//------------------------------------------------------------------------------
// operator=
//------------------------------------------------------------------------------
LightStepSpanContext& LightStepSpanContext::operator=(
    LightStepSpanContext&& other) noexcept {
  sampled_ = other.sampled_;
  data_ = std::move(other.data_);
  return *this;
}

//------------------------------------------------------------------------------
// set_baggage_item
//------------------------------------------------------------------------------
void LightStepSpanContext::set_baggage_item(
    opentracing::string_view key, opentracing::string_view value) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& baggage_data = *data_.mutable_baggage();
  baggage_data.insert(StringMap::value_type(key, value));
} catch (const std::exception&) {
  // Drop baggage item upon error.
}

//------------------------------------------------------------------------------
// baggage_item
//------------------------------------------------------------------------------
std::string LightStepSpanContext::baggage_item(
    opentracing::string_view key) const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto& baggage = data_.baggage();
  auto lookup = baggage.find(key);
  if (lookup != baggage.end()) {
    return lookup->second;
  }
  return {};
}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LightStepSpanContext::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  for (const auto& baggage_item : data_.baggage()) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}

//------------------------------------------------------------------------------
// sampled
//------------------------------------------------------------------------------
bool LightStepSpanContext::sampled() const noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return sampled_;
}

//------------------------------------------------------------------------------
// set_sampled
//------------------------------------------------------------------------------
void LightStepSpanContext::set_sampled(bool sampled) noexcept {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  sampled_ = sampled;
}

//------------------------------------------------------------------------------
// operator==
//------------------------------------------------------------------------------
bool operator==(const LightStepSpanContext& lhs,
                const LightStepSpanContext& rhs) {
  auto extract_baggage = [](const LightStepSpanContext& span_context) {
    std::unordered_map<std::string, std::string> baggage;
    span_context.ForeachBaggageItem(
        [&](const std::string& key, const std::string& value) {
          baggage.emplace(key, value);
          return true;
        });
    return baggage;
  };

  return lhs.trace_id() == rhs.trace_id() && lhs.span_id() == rhs.span_id() &&
         lhs.sampled() == rhs.sampled() &&
         extract_baggage(lhs) == extract_baggage(rhs);
}
}  // namespace lightstep
