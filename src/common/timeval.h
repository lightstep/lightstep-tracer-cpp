#pragma once

#ifndef _WINSOCK_H
#ifndef _TIMEVAL_
#define _TIMEVAL_

typedef long int suseconds_t;
typedef long int time_t;

struct timeval {
  time_t tv_sec;       /* Seconds.  */
  suseconds_t tv_usec; /* Microseconds.  */
};
#endif
#endif