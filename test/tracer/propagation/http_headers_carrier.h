#pragma once

#include <unordered_map>

#include <opentracing/propagation.h>

namespace lightstep {
class HTTPHeadersCarrier : public opentracing::HTTPHeadersReader,
                           public opentracing::HTTPHeadersWriter {
 public:
  explicit HTTPHeadersCarrier(
      std::unordered_map<std::string, std::string>& text_map_);

  // opentracing::HTPPHeadersReader
  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override;

  // opentracing::HTTPHeadersWriter
  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override;

 private:
  std::unordered_map<std::string, std::string>& text_map;
};
}  // namespace lightstep
