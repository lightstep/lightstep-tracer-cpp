#include "common/platform/utility.h"

#include <memory>
#include <string>

#include <Windows.h>

namespace lightstep {
std::string GetProgramName() {
  const int path_max = 1024;
  TCHAR exe_path_char[path_max];

  // this returns a DWORD by default
  auto size =
      static_cast<int>(GetModuleFileName(nullptr, exe_path_char, path_max));

  std::string exe_path{exe_path_char};

  if (size <= 0) {
    return "c++-program";  // Dunno...
  }

  size_t lslash = exe_path.rfind('\\');
  if (lslash != std::string::npos) {
    return exe_path.substr(lslash + 1);
  }
  return exe_path;
}
}  // namespace lightstep
