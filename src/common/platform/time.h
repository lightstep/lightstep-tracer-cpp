#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
