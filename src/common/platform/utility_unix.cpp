#include "common/platform/utility.h"

#include <memory>
#include <string>

#include <unistd.h>

namespace lightstep {
std::string GetProgramName() {
  constexpr int path_max = 1024;
  std::unique_ptr<char[]> exe_path{new char[path_max]};
  ssize_t size = ::readlink("/proc/self/exe", exe_path.get(), path_max);
  if (size == -1) {
    return "c++-program";  // Dunno...
  }
  std::string path(exe_path.get(), size);
  size_t lslash = path.rfind('/');
  if (lslash != std::string::npos) {
    return path.substr(lslash + 1);
  }
  return path;
}
}  // namespace lightstep
