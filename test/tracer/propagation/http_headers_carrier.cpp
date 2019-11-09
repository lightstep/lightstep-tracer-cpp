#include "test/tracer/propagation/http_headers_carrier.h"

#include <random>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// RandomlyCapitalize
//--------------------------------------------------------------------------------------------------
static std::string RandomlyCapitalize(opentracing::string_view s) {
  static thread_local std::mt19937 random_number_generator{0};
  std::string result;
  result.reserve(s.size());
  std::bernoulli_distribution distribution{0.5};
  for (auto c : s) {
    if (distribution(random_number_generator)) {
      result.push_back(static_cast<char>(std::toupper(c)));
    } else {
      result.push_back(static_cast<char>(std::tolower(c)));
    }
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
HTTPHeadersCarrier::HTTPHeadersCarrier(
    std::unordered_map<std::string, std::string>& text_map_)
    : text_map(text_map_) {}

//--------------------------------------------------------------------------------------------------
// Set
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> HTTPHeadersCarrier::Set(
    opentracing::string_view key, opentracing::string_view value) const {
  text_map[key] = value;
  return {};
}

//--------------------------------------------------------------------------------------------------
// ForeachKey
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> HTTPHeadersCarrier::ForeachKey(
    std::function<opentracing::expected<void>(opentracing::string_view key,
                                              opentracing::string_view value)>
        f) const {
  for (const auto& key_value : text_map) {
    auto key = RandomlyCapitalize(key_value.first);
    auto result = f(key, key_value.second);
    if (!result) {
      return result;
    }
  }
  return {};
}
}  // namespace lightstep
