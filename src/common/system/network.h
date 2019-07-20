#pragma once

#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

namespace lightstep {
#ifdef _WIN32
using FileDescriptor = intptr_t;
const FileDescriptor InvalidSocket = INVALID_SOCKET;
using IoVec = WSABUF;
// No documented maximum so pick a reasonable value
const int IoVecMax = 1024;
#else
using FileDescriptor = int;
const FileDescriptor InvalidSocket = -1;
using IoVec = iovec;
extern const int IoVecMax;
#endif

IoVec MakeIoVec(void* data, size_t length) noexcept;

int Read(FileDescriptor socket, void* data, size_t size) noexcept;

int WriteV(FileDescriptor socket, const IoVec* iov, int iovcnt) noexcept; 

int SetSocketNonblocking(FileDescriptor socket) noexcept;

int SetSocketReuseAddress(FileDescriptor socket) noexcept;

int CloseSocket(FileDescriptor socket) noexcept;
} // namespace lightstep
