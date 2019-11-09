#pragma once

#include <unordered_map>

#include <opentracing/propagation.h>

namespace lightstep {
struct HTTPHeadersCarrier : opentracing::HTTPHeadersReader,
                            opentracing::HTTPHeadersWriter {
  explicit HTTPHeadersCarrier(
      std::unordered_map<std::string, std::string>& text_map_);

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override;

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override;

  std::unordered_map<std::string, std::string>& text_map;
};
}  // namespace lightstep
