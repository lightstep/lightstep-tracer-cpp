#pragma once

namespace lightstep {
/**
 * Manages the initialization and deinitialization of the process's network
 * environment. On unix systems, this does nothing; on windows, it calls
 * WSAStartup and WSACleanup.
 *
 * See
 * https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
 */
class NetworkEnvironment {
 public:
  NetworkEnvironment();

  ~NetworkEnvironment() noexcept;
};
}  // namespace lightstep
