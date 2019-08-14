#pragma once

namespace lightstep {
/**
 * Portable wrapper to strcasecmp
 * See https://linux.die.net/man/3/strcasecmp
 */
int StrCaseCmp(const char* s1, const char* s2) noexcept;
}  // namespace lightstep

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
