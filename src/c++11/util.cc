#include <unistd.h>

#include <memory>
#include <string>

#include "lightstep/impl.h"
#include "lightstep/util.h"
#include "lightstep/value.h"

namespace lightstep {
namespace util {

// TODO This only works on Linux.
std::string program_name() {
  constexpr int path_max = 1024;
  std::unique_ptr<char[]> exe_path(new char[path_max]);
  ssize_t size = ::readlink("/proc/self/exe", exe_path.get(), path_max);
  if (size == -1) {
    return "c++-program"; // Dunno...
  }
  std::string path(exe_path.get(), size);
  size_t lslash = path.rfind("/");
  if (lslash != path.npos) {
    return path.substr(lslash+1);
  }
  return path;
}

google::protobuf::Timestamp to_timestamp(TimeStamp t) {
  using namespace std::chrono;
  auto nanos = duration_cast<nanoseconds>(t.time_since_epoch()).count();
  google::protobuf::Timestamp ts;
  const uint64_t nanosPerSec = 1000000000;
  ts.set_seconds(nanos / nanosPerSec);
  ts.set_nanos(nanos % nanosPerSec);
  return ts;
}

template <>
collector::KeyValue make_kv(const std::string& key, const uint64_t& value) {
  collector::KeyValue kv;
  kv.set_key(key);
  kv.set_int_value(value);  // NOTE: Sign conversion.
  return kv;
}

template <>
collector::KeyValue make_kv(const std::string& key, const std::string& value) {
  collector::KeyValue kv;
  kv.set_key(key);
  kv.set_string_value(value);
  return kv;
}

namespace {

struct KeyValueVisitor {
  explicit KeyValueVisitor(collector::KeyValue *kv) : kv(kv) { }
  collector::KeyValue* kv;

  template <typename T>
  void operator()(const T& val) const;

  void operator()(int32_t x) const { kv->set_int_value(x); }
  void operator()(int64_t x) const { kv->set_int_value(x); }

  // Note: Sign conversion.
  void operator()(uint32_t x) const { kv->set_int_value(x); }
  void operator()(uint64_t x) const { kv->set_int_value(x); }

  void operator()(float f) const { kv->set_double_value(f); }
  void operator()(double f) const { kv->set_double_value(f); }

  void operator()(bool b) const { kv->set_bool_value(b); }

  void operator()(const std::string &s) const { kv->set_string_value(s); }

  // More-or-less unsupported types:
  void operator()(std::nullptr_t) const { kv->set_string_value("null"); }

  void operator()(const Values &v) const {
    kv->set_string_value("TODO: Values-to-string");
  }

  void operator()(const Dictionary &v) const {
    kv->set_string_value("TODO: Dictionary-to-string");
  }
};

} // namespace

template <>
collector::KeyValue make_kv(const std::string& key, const Value& value) {
  collector::KeyValue kv;
  kv.set_key(key);
  mapbox::util::apply_visitor(KeyValueVisitor(&kv), value);
  return kv;
}

} // namespace util

} // namespace lightstep
