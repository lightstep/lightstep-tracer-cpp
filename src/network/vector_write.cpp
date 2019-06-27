#include "network/vector_write.h"

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include<iostream>

//#include <alloca.h>
//#include <sys/uio.h>

#include "winsock2.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Write
//--------------------------------------------------------------------------------------------------
bool Write(int socket,
           std::initializer_list<FragmentInputStream*> fragment_input_streams) {
  int num_fragments = 0;
  for (auto fragment_input_stream : fragment_input_streams) {
    num_fragments += fragment_input_stream->num_fragments();
  }
  if (num_fragments == 0) {
    return true;
  }
 
		
	// alloca allocates memory on the stack (so it goes away when we leave this block) 
	// WSABUF is a structure that stores char * buf and ulong len
  auto fragments = static_cast<WSABUF*>(alloca(sizeof(WSABUF) * num_fragments));


  auto fragment_iter = fragments;
  for (auto fragment_input_stream : fragment_input_streams) {
    fragment_input_stream->ForEachFragment(
        [&fragment_iter](void* data, int size) {
          *fragment_iter++ = WSABUF{static_cast<unsigned long>(size), static_cast<char*>(data)};
          return true;
        });
  }
  assert(fragment_iter == fragments + num_fragments);

	// there is no well-documented maximum for how many buffers this can send
	// set lpOverlapped and lpCompletionRoutine to NULL so that this function is synchronous
	// (because the unix writev used here before was sync)
	// https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsasend
  
	unsigned long bytes_sent;
	auto rcode = WSASend(socket, fragments, num_fragments, static_cast<LPDWORD>(&bytes_sent), 0, NULL, NULL);

  if (rcode == -1) {
		int error_code = WSAGetLastError();

		// WSAEWOULDBLOCK is returned if an operation can't be completed immediately on a 
		// nonblocking socket. Not fatal, try again later.
    if (error_code == WSAEWOULDBLOCK) {
			std::cout << "unable to write because write would be blocking" << std::endl;
			return false;
		}

    std::ostringstream oss;
    oss << "writev failed: " << std::strerror(error_code);
    throw std::runtime_error{oss.str()};
  }

	std::cout << "successfully wrote " << bytes_sent << " bytes" << std::endl;

  return Consume(fragment_input_streams, static_cast<int>(rcode));
}
}  // namespace lightstep
