#pragma once

#include <unordered_map>

#include <opentracing/propagation.h>

namespace lightstep {
//------------------------------------------------------------------------------
// TextMapCarrier
//------------------------------------------------------------------------------
struct TextMapCarrier : opentracing::TextMapReader, opentracing::TextMapWriter {
  explicit TextMapCarrier(
      std::unordered_map<std::string, std::string>& text_map_);

  // opentracing::TextMapWriter
  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override;

  // opentracing::Text MapReader
  opentracing::expected<opentracing::string_view> LookupKey(
      opentracing::string_view key) const override;

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override;

  bool supports_lookup = false;
  mutable int foreach_key_call_count = 0;
  std::unordered_map<std::string, std::string>& text_map;
};
}  // namespace lightstep
