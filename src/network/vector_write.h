#pragma once

#include <tuple>

#include "common/fragment_input_stream.h"

namespace lightstep {
/**
 * Uses the writev system call to send fragments over a given socket. After
 * writing, consumes the number of written bytes from the streams.
 * @param socket the file descriptor of the socket.
 * @param fragment_input_streams the list of fragments to send.
 * @return true if everything in the streams was written; false, otherwise.
 */
bool Write(int socket,
           std::initializer_list<FragmentInputStream*> fragment_input_streams);
}  // namespace lightstep
