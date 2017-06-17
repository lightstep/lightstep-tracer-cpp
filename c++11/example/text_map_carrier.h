#ifndef LIGHTSTEP_TEXT_MAP_CARRIER
#define LIGHTSTEP_TEXT_MAP_CARRIER

#include <opentracing/propagation.h>
#include <string>
#include <unordered_map>

using opentracing::TextMapReader;
using opentracing::TextMapWriter;
using opentracing::Expected;
using opentracing::StringRef;

class TextMapCarrier : public TextMapReader, public TextMapWriter {
 public:
  TextMapCarrier(std::unordered_map<std::string, std::string>& text_map)
      : text_map_(text_map) {}

  Expected<void> Set(const std::string& key,
                     const std::string& value) const override {
    text_map_[key] = value;
    return {};
  }

  Expected<void> ForeachKey(
      std::function<Expected<void>(StringRef key, StringRef value)> f)
      const override {
    for (const auto& key_value : text_map_) {
      auto result = f(key_value.first, key_value.second);
      if (!result) return result;
    }
    return {};
  }

 private:
  std::unordered_map<std::string, std::string>& text_map_;
};

#endif  // LIGHTSTEP_TEXT_MAP_CARRIER
