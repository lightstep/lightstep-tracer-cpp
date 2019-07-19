#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
using sa_family_t = unsigned int;
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
