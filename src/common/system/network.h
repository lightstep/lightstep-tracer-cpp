#pragma once

#include <cstddef>

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
using sa_family_t = unsigned int;
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

namespace lightstep {
int Read(int socket, void* data, size_t size) noexcept;
} // namespace lightstep
