#pragma once

#include <tuple>

#include "network/fragment_set.h"

namespace lightstep {
/**
 * Uses the writev systme call to send fragments over a given socket.
 * @param socket the file descriptor of the socket.
 * @param fragment_sets the list of fragments to send.
 * @return a tuple identifying the position of the last byte send.
 */
std::tuple<int, int, int> Write(
    int socket, std::initializer_list<const FragmentSet*> fragment_sets);
}  // namespace lightstep
