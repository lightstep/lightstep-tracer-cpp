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

/**
 * Constructs a segment for a writev system call.
 * @param data a pointer to the segment start
 * @param length the length of the segment
 * @return a segment for the sepecified region of memory
 */
IoVec MakeIoVec(void* data, size_t length) noexcept;

/**
 * Portable wrapper for the read system call.
 * See http://man7.org/linux/man-pages/man2/read.2.html
 */
int Read(FileDescriptor socket, void* data, size_t size) noexcept;

/**
 * Portable wrapper for the writev system call.
 * See https://linux.die.net/man/2/writev
 */
int WriteV(FileDescriptor socket, const IoVec* iov, int iovcnt) noexcept;

/**
 * Sets a given socket to be non-blocking.
 * @param socket the socket to set
 * @return 0 on success
 */
int SetSocketNonblocking(FileDescriptor socket) noexcept;

/**
 * Sets a given socket for address reuse
 * @param socket the socket to set
 * @return 0 on success
 */
int SetSocketReuseAddress(FileDescriptor socket) noexcept;

/**
 * Closes a socket
 * @param socket the socket to close
 * @return 0 on success
 */
int CloseSocket(FileDescriptor socket) noexcept;
}  // namespace lightstep
