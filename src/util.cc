#include <unistd.h>

#include <memory>
#include <string>

#include "impl.h"
#include "value.h"

namespace lightstep {
namespace util {

// TODO This works only on UNIX, and is not tested.
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

std::string id_to_string(uint64_t id) {
  const char hex[] = "0123456789abcdef";
  char buf[16];
  for (int i = 0; i < 16; i++) {
    buf[i] = hex[(id >> (i * 4)) & 0xf];
  }
  return std::string(buf, 16);
}

uint64_t to_micros(TimeStamp t) {
  using namespace std::chrono;
  return duration_cast<microseconds>(t.time_since_epoch()).count();
}

} // namespace util

namespace {

// TODO could generate a JSON encoding
struct ValueVisitor {
  explicit ValueVisitor(std::string *str) : str(str) { }
  std::string* str;

  // The numeric types
  template <typename T>
  void operator()(const T& val) const {
    str->append(std::to_string(val));
  }

  void operator()(bool b) const {
    str->append(b ? "true" : "false");
  }
  void operator()(std::nullptr_t) const {
    str->append("null");
  }
  void operator()(const std::string &s) const {
    str->append(s);
  }
  void operator()(const Values &v) const {
    str->append("TODO: Values-to-string");
  }
  void operator()(const Dictionary &v) const {
    str->append("TODO: Dictionary-to-string");
  }
};

} // namespace

std::string Value::to_string() const {
  std::string s;
  mapbox::util::apply_visitor(ValueVisitor(&s), *this);
  return s;
}

} // namespace lightstep
