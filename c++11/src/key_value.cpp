#include <collector.pb.h>
#include <lightstep/rapidjson/stringbuffer.h>
#include <lightstep/rapidjson/writer.h>
#include <opentracing/stringref.h>
#include <opentracing/value.h>
#include <stdexcept>
using namespace opentracing;

namespace lightstep {
namespace {
using JsonWriter = rapidjson::Writer<rapidjson::StringBuffer>;
}  // end anonymous namesapce

//------------------------------------------------------------------------------
// to_json
//------------------------------------------------------------------------------
static bool to_json(JsonWriter& writer, const Value& value);

namespace {
struct JsonValueVisitor {
  JsonWriter& writer;
  bool result;

  void operator()(bool value) { result = writer.Bool(value); }

  void operator()(double value) { result = writer.Double(value); }

  void operator()(int64_t value) { result = writer.Int64(value); }

  void operator()(uint64_t value) { result = writer.Uint64(value); }

  void operator()(const std::string& s) {
    result = writer.String(s.data(), s.size());
  }

  void operator()(std::nullptr_t) { result = writer.Null(); }

  void operator()(const char* s) { result = writer.String(s); }

  void operator()(const Values& values) {
    if (!(result = writer.StartArray())) return;
    for (const auto& value : values)
      if (!(result = to_json(writer, value))) return;
    result = writer.EndArray();
  }

  void operator()(const Dictionary& dictionary) {
    if (!(result = writer.StartObject())) return;
    for (const auto& key_value : dictionary) {
      if (!(result =
                writer.Key(key_value.first.data(), key_value.first.size())))
        return;
      if (!(result = to_json(writer, key_value.second))) return;
    }
    result = writer.EndObject();
  }
};
}  // anonymous namespace

static bool to_json(JsonWriter& writer, const Value& value) {
  JsonValueVisitor value_visitor{writer, true};
  apply_visitor(value_visitor, value);
  return value_visitor.result;
}

static std::string to_json(const Value& value) {
  rapidjson::StringBuffer buffer;
  JsonWriter writer(buffer);
  if (!to_json(writer, value)) return {};
  return buffer.GetString();
}

//------------------------------------------------------------------------------
// to_key_value
//------------------------------------------------------------------------------
namespace {
struct ValueVisitor {
  collector::KeyValue& key_value;
  const Value& original_value;

  void operator()(bool value) const { key_value.set_bool_value(value); }

  void operator()(double value) const { key_value.set_double_value(value); }

  void operator()(int64_t value) const { key_value.set_int_value(value); }

  void operator()(uint64_t value) const {
    // There's no uint64_t value type so cast to an int64_t.
    key_value.set_int_value(static_cast<int64_t>(value));
  }

  void operator()(const std::string& s) const { key_value.set_string_value(s); }

  void operator()(std::nullptr_t) const { key_value.set_bool_value(false); }

  void operator()(const char* s) const { key_value.set_string_value(s); }

  void operator()(const Values& values) const {
    key_value.set_json_value(to_json(original_value));
  }

  void operator()(const Dictionary& dictionary) const {
    key_value.set_json_value(to_json(original_value));
  }
};
}  // anonymous namespace

collector::KeyValue to_key_value(StringRef key, const Value& value) {
  collector::KeyValue key_value;
  key_value.set_key(key);
  ValueVisitor value_visitor{key_value, value};
  apply_visitor(value_visitor, value);
  return key_value;
}
}  // namespace lightstep
