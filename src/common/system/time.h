#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
