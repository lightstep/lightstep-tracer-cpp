#pragma once

#include <string>
#include <vector>

namespace lightstep {
/**
 * Manages a child process.
 */
class ChildProcessHandle {
 public:
  ChildProcessHandle() noexcept = default;

  ChildProcessHandle(const char* command,
                     const std::vector<std::string>& arguments);

  ChildProcessHandle(const ChildProcessHandle&) = delete;
  ChildProcessHandle(ChildProcessHandle&& other) noexcept;

  ~ChildProcessHandle() noexcept;

  ChildProcessHandle& operator=(const ChildProcessHandle&) = delete;
  ChildProcessHandle& operator=(ChildProcessHandle&& other) noexcept;

 private:
  int pid_{-1};

  void KillChild() noexcept;
};
}  // namespace lightstep
