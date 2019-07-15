#include "test/child_process_handle.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <iterator>

#include <sys/types.h>

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sstream>


namespace lightstep {
//--------------------------------------------------------------------------------------------------
// StartChild
//--------------------------------------------------------------------------------------------------

// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
// returns process handle
static int StartChild(const char* command,
                      const std::vector<std::string>& arguments) {

  std::stringstream full_command;

  full_command << command;

  for (auto arg_p = arguments.begin(); arg_p != arguments.end(); arg_p++) {
    full_command << " " << *arg_p;
  }

  std::cout << "'" << full_command.str() << "'" << std::endl;

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  // TODO: figure out what this does !
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

  // Start the child process.
  if( !CreateProcess( NULL,   // No module name (use command line)
      (LPSTR) full_command.str().c_str(),        // Command line
      NULL,           // Process handle not inheritable
      NULL,           // Thread handle not inheritable
      FALSE,          // Set handle inheritance to FALSE
      0,              // No creation flags
      NULL,           // Use parent's environment block
      NULL,           // Use parent's starting directory
      &si,            // Pointer to STARTUPINFO structure
      &pi )           // Pointer to PROCESS_INFORMATION structure
  )
  {
      std::cerr << "CreateProcess failed " <<  GetLastError() << std::endl;
      return -1;
  }

  return (int) pi.dwProcessId;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ChildProcessHandle::ChildProcessHandle(
    const char* command, const std::vector<std::string>& arguments) {
  pid_ = StartChild(command, arguments);

  std::cout << "pid = " << pid_ << std::endl;
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

  // get process handle from PID
  HANDLE hChildProc = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid_);

  if (hChildProc) {
    ::TerminateProcess(hChildProc, 1);
    ::CloseHandle(hChildProc);
  } else {
    std::cerr << "there was a problem finding the child process" << std::endl;
  }

  std::cout << "started shutting down child process" << std::endl;

  // blocks until child process exist
  WaitForSingleObject( hChildProc, INFINITE );

  std::cout << "successfully shutdown child process" << std::endl;

  pid_ = -1; // so that we know this ChildProcessHandle doesn't work
}
}  // namespace lightstep
