#pragma once

namespace lightstep {
/**
 * Portable wrapper for pthread_atfork.
 * See
 * https://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_atfork.html
 */
int AtFork(void (*prepare)(), void (*parent)(), void (*child)()) noexcept;
}  // namespace lightstep
