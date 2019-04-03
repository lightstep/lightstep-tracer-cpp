#ifndef LIGHTSTEP_TEXT_MAP_CARRIER
#define LIGHTSTEP_TEXT_MAP_CARRIER

#include <opentracing/propagation.h>
#include <string>
#include <unordered_map>

class TextMapCarrier : public opentracing::TextMapReader,
                       public opentracing::TextMapWriter {
 public:
  TextMapCarrier(std::unordered_map<std::string, std::string>& text_map)
      : text_map_(text_map) {}

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override {
    text_map_[key] = value;
    return {};
  }

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override {
    for (const auto& key_value : text_map_) {
      auto result = f(key_value.first, key_value.second);
      if (!result) {
        return result;
      }
    }
    return {};
  }

 private:
  std::unordered_map<std::string, std::string>& text_map_;
};

#endif  // LIGHTSTEP_TEXT_MAP_CARRIER
