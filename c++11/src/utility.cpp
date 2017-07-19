#include "utility.h"
#include <collector.pb.h>
#include <lightstep/rapidjson/stringbuffer.h>
#include <lightstep/rapidjson/writer.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <unistd.h>
#include <stdexcept>
using namespace opentracing;

namespace lightstep {
namespace {
using JsonWriter = rapidjson::Writer<rapidjson::StringBuffer>;
}  // namespace

//------------------------------------------------------------------------------
// ToTimestamp
//------------------------------------------------------------------------------
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t) {
  using namespace std::chrono;
  auto nanos = duration_cast<nanoseconds>(t.time_since_epoch()).count();
  google::protobuf::Timestamp ts;
  const uint64_t nanosPerSec = 1000000000;
  ts.set_seconds(nanos / nanosPerSec);
  ts.set_nanos(nanos % nanosPerSec);
  return ts;
}

//------------------------------------------------------------------------------
// GenerateId
//------------------------------------------------------------------------------
uint64_t GenerateId() {
  static thread_local std::mt19937_64 rand_source{std::random_device()()};
  return rand_source();
}

//------------------------------------------------------------------------------
// GetProgramName
//------------------------------------------------------------------------------
std::string GetProgramName() {
  constexpr int path_max = 1024;
  std::unique_ptr<char[]> exe_path(new char[path_max]);
  ssize_t size = ::readlink("/proc/self/exe", exe_path.get(), path_max);
  if (size == -1) {
    return "c++-program";  // Dunno...
  }
  std::string path(exe_path.get(), size);
  size_t lslash = path.rfind('/');
  if (lslash != path.npos) {
    return path.substr(lslash + 1);
  }
  return path;
}

//------------------------------------------------------------------------------
// ToJson
//------------------------------------------------------------------------------
static bool ToJson(JsonWriter& writer, const Value& value);

namespace {
struct JsonValueVisitor {
  JsonWriter& writer;
  bool result;

  void operator()(bool value) { result = writer.Bool(value); }

  void operator()(double value) { result = writer.Double(value); }

  void operator()(int64_t value) { result = writer.Int64(value); }

  void operator()(uint64_t value) { result = writer.Uint64(value); }

  void operator()(const std::string& s) {
    result = writer.String(s.data(), static_cast<unsigned>(s.size()));
  }

  void operator()(std::nullptr_t) { result = writer.Null(); }

  void operator()(const char* s) { result = writer.String(s); }

  void operator()(const Values& values) {
    if (!(result = writer.StartArray())) {
      return;
    }
    for (const auto& value : values) {
      if (!(result = ToJson(writer, value))) {
        return;
      }
    }
    result = writer.EndArray();
  }

  void operator()(const Dictionary& dictionary) {
    if (!(result = writer.StartObject())) {
      return;
    }
    for (const auto& key_value : dictionary) {
      if (!(result =
                writer.Key(key_value.first.data(),
                           static_cast<unsigned>(key_value.first.size())))) {
        return;
      }
      if (!(result = ToJson(writer, key_value.second))) {
        return;
      }
    }
    result = writer.EndObject();
  }
};
}  // anonymous namespace

static bool ToJson(JsonWriter& writer, const Value& value) {
  JsonValueVisitor value_visitor{writer, true};
  apply_visitor(value_visitor, value);
  return value_visitor.result;
}

static std::string ToJson(const Value& value) {
  rapidjson::StringBuffer buffer;
  JsonWriter writer(buffer);
  if (!ToJson(writer, value)) {
    return {};
  }
  return buffer.GetString();
}

//------------------------------------------------------------------------------
// ToKeyValue
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

  void operator()(const Values& /*unused*/) const {
    key_value.set_json_value(ToJson(original_value));
  }

  void operator()(const Dictionary& /*unused*/) const {
    key_value.set_json_value(ToJson(original_value));
  }
};
}  // anonymous namespace

collector::KeyValue ToKeyValue(string_view key, const Value& value) {
  collector::KeyValue key_value;
  key_value.set_key(key);
  ValueVisitor value_visitor{key_value, value};
  apply_visitor(value_visitor, value);
  return key_value;
}
}  // namespace lightstep
