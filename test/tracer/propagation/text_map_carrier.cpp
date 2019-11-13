#include "test/tracer/propagation/text_map_carrier.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
TextMapCarrier::TextMapCarrier(
    std::unordered_map<std::string, std::string>& text_map_)
    : text_map(text_map_) {}

//--------------------------------------------------------------------------------------------------
// Set
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> TextMapCarrier::Set(
    opentracing::string_view key, opentracing::string_view value) const {
  text_map[key] = value;
  return {};
}

//--------------------------------------------------------------------------------------------------
// LookupKey
//--------------------------------------------------------------------------------------------------
opentracing::expected<opentracing::string_view> TextMapCarrier::LookupKey(
    opentracing::string_view key) const {
  if (!supports_lookup) {
    return opentracing::make_unexpected(
        opentracing::lookup_key_not_supported_error);
  }
  auto iter = text_map.find(key);
  if (iter != text_map.end()) {
    return opentracing::string_view{iter->second};
  }
  return opentracing::make_unexpected(opentracing::key_not_found_error);
}

//--------------------------------------------------------------------------------------------------
// ForeachKey
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> TextMapCarrier::ForeachKey(
    std::function<opentracing::expected<void>(opentracing::string_view key,
                                              opentracing::string_view value)>
        f) const {
  ++foreach_key_call_count;
  for (const auto& key_value : text_map) {
    auto result = f(key_value.first, key_value.second);
    if (!result) {
      return result;
    }
  }
  return {};
}
}  // namespace lightstep
