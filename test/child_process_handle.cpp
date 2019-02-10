#include "test/child_process_handle.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <iterator>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// StartChild
//--------------------------------------------------------------------------------------------------
static int StartChild(const char* command,
                      const std::vector<std::string>& arguments) {
  std::vector<const char*> argv;
  argv.reserve(arguments.size() + 2);
  argv.push_back(command);
  std::transform(arguments.begin(), arguments.end(), std::back_inserter(argv),
                 [](const std::string& s) { return s.data(); });
  argv.push_back(nullptr);
  auto rcode = ::fork();
  if (rcode == -1) {
    std::cerr << "fork failed: " << std::strerror(errno) << "\n";
    std::terminate();
  }
  if (rcode != 0) {
    return rcode;
  }
  ::execv(command, const_cast<char* const*>(argv.data()));

  // Only run this code if execv failed
  std::cerr << "execv failed: " << std::strerror(errno) << "\n";
  std::terminate();
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ChildProcessHandle::ChildProcessHandle(
    const char* command, const std::vector<std::string>& arguments) {
  pid_ = StartChild(command, arguments);
}

ChildProcessHandle::ChildProcessHandle(ChildProcessHandle&& other) noexcept {
  pid_ = other.pid_;
  other.pid_ = -1;
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
ChildProcessHandle::~ChildProcessHandle() noexcept { KillChild(); }

//--------------------------------------------------------------------------------------------------
// operator=
//--------------------------------------------------------------------------------------------------
ChildProcessHandle& ChildProcessHandle::operator=(
    ChildProcessHandle&& other) noexcept {
  KillChild();
  pid_ = other.pid_;
  other.pid_ = -1;
  return *this;
}

//--------------------------------------------------------------------------------------------------
// KillChild
//--------------------------------------------------------------------------------------------------
void ChildProcessHandle::KillChild() noexcept {
  if (pid_ == -1) {
    return;
  }
  // Note: We do a hard kill here because TSAN does signal handling interception
  // that prevents the child process from receiving an ordinary SIGTERM.
  //
  // See https://github.com/google/sanitizers/issues/838
  auto rcode = kill(pid_, SIGKILL);
  if (rcode != 0) {
    std::cerr << "failed to kill child process: " << std::strerror(errno)
              << "\n";
    std::terminate();
  }

  int status;
  rcode = ::waitpid(pid_, &status, 0);
  if (rcode != pid_) {
    std::cerr << "failed to kill child: waitpid returned " << rcode << "\n";
    std::terminate();
  }
  pid_ = -1;
}
}  // namespace lightstep
