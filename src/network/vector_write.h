#pragma once

#include <tuple>

#include "network/fragment_input_stream.h"

namespace lightstep {
/**
 * Uses the writev system call to send fragments over a given socket.
 * @param socket the file descriptor of the socket.
 * @param fragment_input_streams the list of fragments to send.
 * @return a tuple identifying the position of the last byte send.
 */
std::tuple<int, int, int> Write(
    int socket,
    std::initializer_list<const FragmentInputStream*> fragment_input_streams);
}  // namespace lightstep
