#include "common/system/network_environment.h"

#include <stdexcept>
#include <sstream>

#include "common/system/error.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
NetworkEnvironment::NetworkEnvironment() {
  // See https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
  auto winsock_version_requested = MAKEWORD(2, 2);
  WSADATA wsa_data;
  auto rcode = WSAStartup(winsock_version_requested, &wsa_data);
  if (rcode != 0) {
    std::ostringstream oss;
    oss << "WSAStartup failed: " << GetErrorCodeMessage(static_cast<ErrorCode>(rcode));
    throw std::runtime_error{oss.str()};
  }
  if (LOBYTE(wsa_data.wVersion != 2)|| HIBYTE(wsa_data.wVersion) != 2) {
    WSACleanup();
    throw std::runtime_error{"Could not find a suitable Winsock.dll"};
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
NetworkEnvironment::~NetworkEnvironment() noexcept {
  WSACleanup();
}
} // namespace lightstep

